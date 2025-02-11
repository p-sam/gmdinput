#include <stdio.h>
#include <string.h>
#include <sdlmapping.h>
#include <ctype.h>

static inline uint16_t _bswap16(uint16_t x) {
	return (x >> 8) | (x << 8);
}

static const char* _range_str(uint8_t range) {
	switch (range) {
	case SDLMAPPING_RANGE_FULL:
		return " ";
	case SDLMAPPING_RANGE_POSITIVE:
		return "+";
	case SDLMAPPING_RANGE_NEGATIVE:
		return "-";
	default:
		return "?";
	}
}

static const char* _hat_str(uint8_t hat) {
	switch (hat) {
	case SDLMAPPING_HAT_UP:
		return "U";
	case SDLMAPPING_HAT_RIGHT:
		return "R";
	case SDLMAPPING_HAT_DOWN:
		return "D";
	case SDLMAPPING_HAT_LEFT:
		return "L";
	default:
		return "?";
	}
}

static void _print_bind(const SDLMappingBind* bind, uint8_t range, const char* name) {
	if (bind->type == SDLMAPPING_BIND_NONE) {
		return;
	}

	printf("   %s%-16s= ", _range_str(range), name);
	switch (bind->type) {
	case SDLMAPPING_BIND_BUTTON:
		printf("btn[%d]", bind->index);
		break;
	case SDLMAPPING_BIND_AXIS:
		printf("axis[%d]%s%s", bind->index, _range_str(bind->axis.range), bind->axis.invert ? "~" : "");
		break;
	case SDLMAPPING_BIND_HAT:
		printf("hat[%d][%s]", bind->index, _hat_str(bind->hat));
		break;
	default:
		printf("???");
		break;
	}
	printf("\n");
}

static void _gen_header_bind(FILE* fout, SDLMappingBind* bind, const char* name, const char* indent) {
	fprintf(fout, "%s/* %s */{\n", indent, name);

	fprintf(fout, "%s\t.type = %u,\n", indent, bind->type);

	if (bind->type != SDLMAPPING_BIND_NONE) {
		fprintf(fout, "%s\t.index = %u,\n", indent, bind->index);
	}

	if (bind->type == SDLMAPPING_BIND_HAT) {
		fprintf(fout, "%s\t.hat = %u,\n", indent, bind->hat);
	}

	if (bind->type == SDLMAPPING_BIND_AXIS) {
		fprintf(fout, "%s\t.axis = {\n", indent);

		fprintf(fout, "%s\t\t.range = %d,\n", indent, bind->axis.range);
		fprintf(fout, "%s\t\t.invert = %u,\n", indent, bind->axis.invert);

		fprintf(fout, "%s\t},\n", indent);
	}

	fprintf(fout, "%s},\n", indent);
}

static int _gen_header(FILE* fin, FILE* fout) {

	char line[0x1000];
	size_t exported = 0;
	fprintf(fout, "#pragma once\n");
	fprintf(fout, "// GENERATED FILE\n\n");
	fprintf(fout, "#include <sdlmapping.h>\n\n");
	fprintf(fout, "typedef struct { unsigned long pidvid; const char* guid; const char* name; SDLMappingBinds binds; } SDLGenMapping;\n\n");
	fprintf(fout, "static SDLGenMapping g_mappings[] = {\n");

	while (fgets(line, sizeof(line), fin)) {
		size_t len = strcspn(line, "\r\n");
		if (len >= sizeof(line) - 1) {
			fprintf(stderr, "[!] line exceeded buffer size (%lld)\n", len);
			return 1;
		}
		line[len] = 0;

		if (line[0] != '0' && line[1] != '3') {
			continue;
		}

		SDLMapping mapping;
		size_t pos;
		uint8_t rc = sdlmapping_parse(line, &mapping, &pos);

		if (rc != SDLMAPPING_ERROR_OK) {
			fprintf(stderr, "[!] parsing failed\n");
			fprintf(stderr, "line = [%03llu] '%s'\n", len, line);
			fprintf(stderr, "rc = %d\n", rc);
			fprintf(stderr, "pos = %lld\n", pos);
			return rc;
		}

		if (_stricmp(mapping.platform, "windows") != 0) {
			continue;
		}

		uint16_t pid = 0;
		uint16_t vid = 0;
		int read = sscanf(mapping.guid, "03000000%04hX0000%04hX000000000000", &vid, &pid);
		if (read != 2 || (vid == 0) && (pid == 0)) {
			continue;
		}
		exported++;

		fprintf(fout, "\t{\n");
		fprintf(fout, "\t\t.pidvid = 0x%04hX%04hX, .guid = \"%.*s\",\n", _bswap16(pid), _bswap16(vid), (int)sizeof(mapping.guid), mapping.guid);
		fprintf(fout, "\t\t.name = \"");
		const char* p = mapping.name;
		while (*p) {
			if (iscntrl(*p) || *p == '\\' || *p == '\"') {
				fprintf(fout, "\\%03o", *p);
			} else {
				fputc(*p, fout);
			}
			p++;
		}
		fprintf(fout, "\",\n");
		fprintf(fout, "\t\t.binds = {\n");

		fprintf(fout, "\t\t\t.buttons = {\n");
		for (uint8_t i = 0; i < SDLMAPPING_BUTTON_ENUMCOUNT; i++) {
			_gen_header_bind(fout, &mapping.binds.buttons[i], sdlmapping_button_name(i), "\t\t\t\t");
		}
		fprintf(fout, "\t\t\t},\n");

		fprintf(fout, "\t\t\t.axes = {\n");
		for (uint8_t i = 0; i < SDLMAPPING_AXIS_ENUMCOUNT; i++) {
			fprintf(fout, "\t\t\t\t/* %s */{{\n", sdlmapping_axis_name(i));

			for (uint8_t j = 0; j < SDLMAPPING_RANGE_ENUMCOUNT; j++) {
				_gen_header_bind(fout, &mapping.binds.axes[i].r[j], _range_str(j), "\t\t\t\t\t");
			}

			fprintf(fout, "\t\t\t\t}},\n");
		}
		fprintf(fout, "\t\t\t},\n");

		fprintf(fout, "\t\t},\n");
		fprintf(fout, "\t},\n");
	}

	fprintf(fout, "};\n\n");
	fprintf(fout, "static size_t g_mappings_count = %lld;\n\n", exported);

	printf("%lld mappings exported\n", exported);
	return 0;
}

static int _usage(const char* argv0) {
	const char* name = strrchr(argv0, '\\');
	if (name) {
		name++;
	}
	else {
		name = argv0;
	}
	fprintf(stderr, "usage: %s <cmd> [args]\n\n", name);
	fprintf(stderr, "cmds:\n");
	fprintf(stderr, "    desc <mapping>\n");
	fprintf(stderr, "    header <in.txt> <out.h>\n");
	return 1;
}

static int _cmd_desc(int argc, char** argv) {
	if (argc != 3) {
		return _usage(argv[0]);
	}

	SDLMapping mapping;
	size_t pos;
	uint8_t rc = sdlmapping_parse(argv[2], &mapping, &pos);

	if (rc != SDLMAPPING_ERROR_OK) {
		fprintf(stderr, "[!] parsing failed\n");
		fprintf(stderr, "rc = %d\n", rc);
		fprintf(stderr, "pos = %lld\n", pos);
		return rc;
	}

	printf("guid = %.*s\n", SDLMAPPING_GUID_SIZE, mapping.guid);
	printf("name = '%s'\n", mapping.name);
	printf("platform = '%s'\n", mapping.platform);

	printf("axes:\n");
	for (uint8_t i = 0; i < SDLMAPPING_AXIS_ENUMCOUNT; i++) {
		for (uint8_t j = 0; j < SDLMAPPING_RANGE_ENUMCOUNT; j++) {
			_print_bind(&mapping.binds.axes[i].r[j], j, sdlmapping_axis_name(i));
		}
	}

	printf("buttons:\n");
	for (uint8_t i = 0; i < SDLMAPPING_BUTTON_ENUMCOUNT; i++) {
		_print_bind(&mapping.binds.buttons[i], 0, sdlmapping_button_name(i));
	}

	printf("\n");

	return 0;
}

static int _cmd_header(int argc, char** argv) {
	if (argc != 4) {
		return _usage(argv[0]);
	}

	FILE* fin = fopen(argv[2], "r");
	if (!fin) {
		fprintf(stderr, "[!] in file open fail\n");
		return 1;
	}

	FILE* fout = fopen(argv[3], "w");
	if (!fout) {
		fprintf(stderr, "[!] out file open fail\n");
		return 1;
	}

	int rc = _gen_header(fin, fout);

	fclose(fin);
	fclose(fout);

	return rc;
}

int main(int argc, char** argv) {
	if (argc >= 2) {
		if (strcmp(argv[1], "desc") == 0) {
			return _cmd_desc(argc, argv);
		}

		if (strcmp(argv[1], "header") == 0) {
			return _cmd_header(argc, argv);
		}
	}

	return _usage(argv[0]);
}
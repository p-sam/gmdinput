import fs from 'fs';
import path from 'path';
import assert from 'assert';
import {Yy} from '@bscotch/yy';
import recursiveCopy from 'recursive-copy';

const SRC_DIR = path.resolve(__dirname, 'gms');
const OUT_DIR = path.resolve(__dirname, '../build/ext');
const EXT_NAME = 'gmdinput';
const EXT_VERSION = '0.1.0';
const EXT_PUBLISHER = 'p-sam';
const EXT_FN_PREFIX_FILTER = `${EXT_NAME}_gamepad_`;
const EXT_FN_PREFIX_REPLACE = 'gamepad_';

const RESOURCES = [
    `extensions/${EXT_NAME}`
];

fs.mkdirSync(OUT_DIR, {recursive: true});

await Promise.all(RESOURCES.map(resource => {
    const p = recursiveCopy(path.join(SRC_DIR, resource), path.join(OUT_DIR, resource), {
        overwrite: true,
        dot: true,
        filter: [
            '**/*.dll',
            '**/*.gml',
            '**/*.yy',
        ]
    });
    p.on(recursiveCopy.events.COPY_FILE_START, ({dest}) => console.log(dest));
    return p;
}));

const projectYYP = Yy.readSync(path.resolve(SRC_DIR, './GmsDemo.yyp'));
projectYYP.name = EXT_NAME;
for(const k of Object.keys(projectYYP)) {
    if(!Array.isArray(projectYYP[k])) {
        continue;
    }

    if(k.toLowerCase() !== 'resources') {
        projectYYP[k] = [];
        continue;
    }

    projectYYP[k] = projectYYP[k].filter(r => RESOURCES.some(p => r.id.path.startsWith(p)));
}

Object.assign(projectYYP.MetaData, {
    PackageType: 'Asset',
    PackageName: EXT_NAME,
    PackageID: EXT_NAME,
    PackagePublisher: EXT_PUBLISHER,
    PackageVersion: EXT_VERSION,
});

fs.writeFileSync(path.join(OUT_DIR, 'manifest.json'), JSON.stringify({
    package_id: EXT_NAME,
    display_name: EXT_NAME,
    version: EXT_VERSION,
    package_type: 'asset',
    ide_version: projectYYP.MetaData.IDEVersion
}, null, 4), {encoding: 'utf-8'});

Yy.writeSync(path.join(OUT_DIR, EXT_NAME+'.yyp'), projectYYP);

if(process.argv[2] !== 'unprefixed') {
    process.exit(0);
}

console.log('- unprefixing -');
const extYYPath = path.resolve(OUT_DIR, `extensions/${EXT_NAME}/${EXT_NAME}.yy`);
const extYY = Yy.readSync(extYYPath);

const extFile = extYY.files.find(f => f.filename === `${EXT_NAME}.dll`);
assert(extFile !== null);
for(const fn of extFile.functions) {
    if(!fn.name.startsWith(EXT_FN_PREFIX_FILTER)) {
        continue;
    }
    const oldName = fn.name;
    const newName = EXT_FN_PREFIX_REPLACE + fn.name.slice(EXT_FN_PREFIX_FILTER.length);
    console.log(`renaming '${oldName}' to '${newName}'`);
    fn.name = newName;
}

for(const o of extFile.order) {
    if(!o.name.startsWith(EXT_FN_PREFIX_FILTER)) {
        continue;
    }
    o.name = EXT_FN_PREFIX_REPLACE + o.name.slice(EXT_FN_PREFIX_FILTER.length);
}


Yy.writeSync(extYYPath, extYY);
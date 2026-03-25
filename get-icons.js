import fs from 'node:fs/promises'
import { execSync } from 'node:child_process'
const iconMap = await fs
  .readFile('./owm-code-weathericons-map.json')
  .then(JSON.parse)

fs.stat('./icons').catch(() => {
  fs.mkdir('./icons')
})

console.log('Converting to png')
execSync(
  'for file in ./svg/*.svg; do convert -size 90x90 "$file" -channel RGB -negate "./icons/$(basename -s ".svg" "$file").png"; done',
)

console.log('Mapping to codes')
for (let [id, { icon }] of Object.entries(iconMap)) {
  if (!(id > 699 && id < 800) && !(id > 899 && id < 1000)) {
    await fs
      .copyFile(`./icons/wi-day-${icon}.png`, `./icons/${id}d.png`)
      .catch((e) => console.error('Failed to copy icon', e.code, e.message))
    await fs
      .copyFile(`./icons/wi-night-${icon}.png`, `./icons/${id}n.png`)
      .catch((e) => console.error('Failed to copy icon', e.code, e.message))
  } else {
    await fs.copyFile(`./icons/wi-${icon}.png`, `./icons/${id}.png`)
  }
}

console.log('Done')

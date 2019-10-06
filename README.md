# mongoose-os-libs

## Supported sensors:
- SHT10, SHT11, SHT15 - Temperature 

## How to use:
- Clone this repository.
- Copy needed library to `<Your app folder>/deps/` (if `deps` folder doesn't exist, create it).
- Add to your app mos.yml, under libs:
```
libs:
  - name: <name of coppied library>
```

For example:
```
libs:
  - name: sht1x
```

- Do a local build:
```
mos build --local --platform <your board>
```
or do online build:
```
mos build --plaform <your board>
```

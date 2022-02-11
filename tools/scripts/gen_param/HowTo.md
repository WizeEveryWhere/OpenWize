# How to

## Generate the parameters tables
Just call gen_table.sh as :
```
./gen_table.sh --in ParamsFile.xml --dest the/dest/path
```

The following 3 files will be generated : 
```
parameters_cfg.c
parameters_cfg.h
parameters_default.c
```

Note : the ".h" and ".c" files will be installed into the given "the/dest/path/gen" 
destination path, with the addition of "gen" sub-folder.


If multiple xml were required (access and restriction) then :

1) install ubuntu "gwenhywfar-tools" package (just to get xmlmerge...!)

2) Then, merge them together

```
xmlmerge Params.xml Restriction.xml -o test_conbine.xml
```


As an example, execute the following  :
```
xmlmerge ./example/DefaultParams.xml ./example/DefaultRestr.xml -o ./example/MergedParam.xml
```

Then :
```
./gen_table.sh --in ./example/MergedParam.xml --dest ./my_param
```

In "my_param/gen" directory, the following 3 files are generated : 
```
parameters_cfg.c
parameters_cfg.h
parameters_default.c
```
The directory "my_param" will be created if it doesn't exist yet.

## Some help on xmlstartlet

### Validate
```
xmlstarlet val -e -q --xsd parameters.xsd your_file.xml
```

#### Get the parameters value:
```
xmlstarlet sel -t -v '//Parameter[@id="1E"]' -n DefaultParams.xml
```

#### Get the parameters attibutes:
```
xmlstarlet sel -t -v '//Parameter[@id="3E"]/@name' -n DefaultParams.xml
xmlstarlet sel -t -v '//Parameter[@id="3E"]/@size' -n DefaultParams.xml
xmlstarlet sel -t -v '//Parameter[@id="3E"]/@loc' -n DefaultParams.xml
xmlstarlet sel -t -v '//Parameter[@id="3E"]/@rem' -n DefaultParams.xml
xmlstarlet sel -t -v '//Parameter[@id="3E"]/@desc' -n DefaultParams.xml
```

#### Get all parameters id
```
xmlstarlet sel -t -v '//Parameter/@id' -n DefaultParams_LAN.xml
```

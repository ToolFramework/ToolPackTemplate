# ToolPack

ToolPack is a pack of useful tools to extend the framework and provide extra functionality. This was done to keep the main core lightweight and allow people to add only the parts which are needed.

# Installation

To make use of ToolPack or any other Tool collections navigate to the imported Tools folder In your ToolApplication isntallation.

``` cd Usertools/ImportedTools``` 

Then clone this repo to that location

``` git clone https://github.com/ToolFramework/ToolPack.git ```

Once cloned navigate into the new ToolPack folder and run the ```Import.sh``` script

```
cd ToolPack
./Import.sh
```

```Import.sh``` will open up a graphical dialog if the package is installed allowing you to choose which Tools to import. 

If not you can pass ```Import.sh <option>``` where the option is either the name of an individual Tool you wish to import or "All" to import everything.

Once imported all Tools will be placed in the InactiveTool folder and not compiled with you code unless you activate them using the ```ToolSelect.sh``` script in the ```UserTools``` folder.

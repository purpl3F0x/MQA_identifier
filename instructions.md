# Instructions 

 - For windows in search type *"cmd"* and open the application.  For
   MacOS press **Command** + **Space** and type *"terminal"*.
   
  
  
 - type `cd Downloads` (or the location you downloaded the app)
![](https://i.imgur.com/hKOhgld.png)

- `mqa_identifer.exe {paths to files or folders}...` for Windows or`./mqa_identifier` on OSX 
 
 For example
>  ` mqa_identifer.exe "E:\Music\Deep Purple - Machine Head\03 Pictures of Home.flac" "D:\Music\1000mods - Super Van Vacation (2011) [FLAC-CD]"`

Gives output: 
```
**************************************************
***********  MQA flac identifier tool  ***********
********  Stavros Avramidis (@purpl3F0x)  ********
** https://github.com/purpl3F0x/MQA_identifier  **
**************************************************
Found 11 file for scanning...

  #     Encoding        Name
  1     MQA 96K         03 Pictures of Home.flac
  2     NOT MQA         01 1000mods - Road To Burn.flac
  3     NOT MQA         02 1000mods - 7 Flies.flac
  4     NOT MQA         03 1000mods - El Rollito.flac
  5     NOT MQA         04 1000mods - Set You Free.flac
  6     NOT MQA         05 1000mods - Vidage.flac
  7     NOT MQA         06 1000mods - Navy In Alice.flac
  8     NOT MQA         07 1000mods - Track Me.flac
  9     NOT MQA         08 1000mods - Johny's.flac
 10     NOT MQA         09 1000mods - Abell 1835.flac
 11     NOT MQA         10 1000mods - Super Van Vacation.flac

**************************************************
Scanned 11 files
Found 1 MQA files

Process finished with exit code 0
```
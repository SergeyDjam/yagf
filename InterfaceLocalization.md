# Translating YAGF Interface Into Other Languages #

Since YAGF is written in Qt following the internationalization guidelines, it should be easy to localize its interface for those acquainted with the Qt localization. But since some users wishing to contribute to the YAGF development by providing interface localization seem to be unfamiliar with the process, here is a short guideline on how to translate the YAGF interface. In order to perform translation Qt development tools must be installed in your system. You will also need the YAGF source code (although you will not have to modify the code itself).

## When it is better to perform translation ##

As it happens, most of the users provide their localization files some time after the new release is made. This puts me in a difficult situation as I cannot add files to the release that is already frozen and since the YAGF interface is still undergoing some change your localization may be outdated for the next release. So if you want your localization to appear in the future releases, please let me know that you are interested. I will send you the sources before the release and set the date before which the localization should be ready. Actually when you learn the tools the localization process takes no more than half an hour.

## How to translate ##

In order to localize the interface and make it available to the other users you have to provide the translation source (ts) file for your language and send it to me. All the currently available ts files are stored in src/translations folder. The name of the ts file should look like yagf\_xx.ts where xx is the code for your language. You can find language codes here: http://www.loc.gov/standards/iso639-2/php/code_list.php

There are two localization cases. The first one is when a ts file for the target language already exists, but needs an update as the interface has changed. In this case you should run src/translations/ts\_update.sh.
In case the ts file for your language doesn't exist you should create it with
```
$ touch yagf_xx.ts
```
and then edit ts\_update.sh. Change the portion of the file that looks like
```
-ts yagf_ru.ts yagf_lt.ts yagf_de.ts yagf_pl.ts yagf_uk.ts
```
to
```
-ts yagf_ru.ts yagf_lt.ts yagf_de.ts yagf_pl.ts yagf_uk.ts yagf_xx.ts
```
Now run ts\_update.sh.
The following step in both cases is to edit the ts file in Qt Linguist application:
```
$ linguist yagf_xx.ts
```
Qt Linguist is a graphics app that is easy to use and is well documented (http://qt-project.org/doc/qt-4.8/linguist-translators.html) so I will not describe it here. After you have finished editing your ts file congratulate yourself as your translation source is ready.

## Testing your interface translation ##

Compile your ts file into qm file:
```
$ lrelease yagf_xx.ts
```
The new file yagf\_xx.qm will appear. Drop it to the place where other yagf`*`.qm files live (somewhere like /usr/share/ directory or the like) and start yagf. If everything went fine the yagf interface should be presented to you in a new language. By default the new language for the interface will be used only if this language is the same as your system locale language.

---

When all is done please send to me your yagf\_xx.ts and ts\_update.sh (if you have modified it).
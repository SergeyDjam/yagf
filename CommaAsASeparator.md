# Introduction #

By default in the  "Only Digits" mode the Tesseract OCR recognizes only period sign (".") as a digital separator like in 3.14. If a comma sign appears in the stream of digits is is often misrecognized as "1". Here is an instruction on how to make comma known as separator when using Tesseract OCR and YAGF.

# Details #

Open console and go to your Tesseract data path (something like /usr/share/tesseract-ocr). If you don't know the Tesseract data path in your system, you can find it in the YAGF Settings dialog. Now enter
```
cd tessdata/configs   
```
The symbols recognized in "digits" mode are stored in the "digits" file. By default its contents looks like:
```
tessedit_char_whitelist 0123456789-.
```
To add comma as a separator simply change it to
```
tessedit_char_whitelist 0123456789-.,
```
By default when recognizing digits Tesseract will place a whitespace _before_ each comma. YAGF corrects this automatically.
One may ask why not to enable YAGF to make this setup via its settings. The reason is that you have to make chafes requiring root access, and it is not YAGF's job to give a user root privileges.
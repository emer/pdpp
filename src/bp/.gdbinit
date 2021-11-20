set print pretty on
directory /usr/include/g++
directory /usr/local/iv/iv/src/include/InterViews
directory /usr/local/iv/iv/src/include/IV-look
directory /usr/local/iv/iv/src/include/IV-X11
directory /usr/local/iv/iv/src/lib/InterViews
directory /usr/local/iv/iv/src/lib/IV-X11
directory /usr/local/pdp++/src
directory /usr/local/pdp++/src/iv_misc
directory /usr/local/pdp++/src/ta_string
directory /usr/local/pdp++/src/ta
directory /usr/local/pdp++/src/ta_misc
directory /usr/local/pdp++/src/css
directory /usr/local/pdp++/src/pdp
directory /usr/local/pdp++/include/LINUX/iv_misc
directory /usr/local/pdp++/include/LINUX/ta_string
directory /usr/local/pdp++/include/LINUX/ta
directory /usr/local/pdp++/include/LINUX/ta_misc
directory /usr/local/pdp++/include/LINUX/css
directory /usr/local/pdp++/include/LINUX/pdp
directory $cdir
define char
print (char *) ((char *) $ + 4)
end
document char
Type char after doing a "p string" to get the chars
end
set demangle-style auto

define src
cd /usr/local/pdp++/src
end
define fixta
src
cd ../ta
make optLib
relink
end


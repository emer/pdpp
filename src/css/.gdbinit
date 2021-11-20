set print pretty on
directory /usr/local/lib/g++-include
directory /usr3/local/iv/iv/src/include/InterViews
directory /usr3/local/iv/iv/src/lib/InterViews
directory /usr3/local/iv/iv/src/include/IV-look
directory $cdir/include/ta
directory $cdir/include/css
directory $cdir/include/ivg
directory $cdir/include
directory $cdir
define char
print (char *) ((char *) $ + 4)
end
document char
Type char after doing a "p string" to get the chars
end
set demangle-style auto
define src
cd /usr3/local/pdpdev/src
end
define relink
src
shell rm pdpshell
make
end
define fixlibs
src
cd ta
make
relink
end

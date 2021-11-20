set print pretty on
directory /usr/local/lib/g++-include
directory /usr/local/iv/iv/src/include/InterViews
directory /usr/local/iv/iv/src/include/IV-look
directory /usr/local/iv/iv/src/include/IV-X11
directory /usr/local/iv/iv/src/lib/InterViews
directory /usr/local/iv/iv/src/lib/IV-X11
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
shell rm SUN4/pdpshell
make
end

define fixta
src
cd ../ta
make Lib
relink
end

define fixcss
src
cd ../css/src
make Lib
relink
end

define fixpdpglyphs
src
cd ../pdpglyphs
make Lib
relink
end

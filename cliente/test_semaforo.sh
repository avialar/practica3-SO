#!/usr/bin/zsh -f
cualquier="c\n"
ingresar="1\nMario\nPerro\n13\nNormal\n45\n35\nM\n$cualquier"
ver="2\n1\nn\n$cualquier"
buscar="4\nM\n$cualquier"
borrar="3\n1\n$cualquier"
salir="5\n"
command="$ingresar$ver$buscar$borrar$salir"

echo -e "$command" | ./cliente > /dev/null

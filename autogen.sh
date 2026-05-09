#!/bin/bash
# Script per generare i file di configurazione Autotools
set -e

echo "Inizializzazione di Autotools..."

# 1. Scansiona le macro di Autoconf (se hai una directory m4)
if [ -d "m4" ]; then
    aclocal -I m4
else
    aclocal
fi

# 2. Genera il file configure
autoconf

libtoolize

# 3. Genera i file Makefile.in da Makefile.am
automake --add-missing --copy

echo "Configurazione Autotools completata. Ora esegui: ./configure"


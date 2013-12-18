Bash completion file will be automatically installed with make install
into `/etc/bash_completion.d/`

To install zsh completion file, either manually copy it to proper directory
(one directory from $fpath), or run this command:

    cp _qupzilla "`echo $fpath | cut -d' ' -f1`/_qupzilla"

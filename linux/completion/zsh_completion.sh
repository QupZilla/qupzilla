#compdef qupzilla

opts=(
    '(-)'{-h,--help}'[print usage help]'
    '(-)'{-a,--authors}'[print QupZilla authors]'
    '(-)'{-v,--version}'[print QupZilla version]'

    '(-)'{-p=,--profile=}'[start with specified profile]'
    '(-)'{-ne,--no-extensions}'[start without extensions]'

    '(-)'{-nt,--new-tab}'[open new tab]'
    '(-)'{-nw,--new-window}'[open new window]'
    '(-)'{-pb,--private-browsing}'[start private browsing]'
    '(-)'{-dm,--download-manager}'[show download manager]'
    '(-)'{-nr,--no-remote}'[open new instance]'
    '(-)'{-ct=,--current-tab=}'[open URL in current tab]'
    '(-)'{-ow=,--open-window=}'[open URL in new window]'
    )

_x_arguments -C $opts

return 0

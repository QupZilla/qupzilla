tell application "Finder"
  tell disk "QupZilla"
    open
      # Set some defaults
      set current view of container window to icon view

      # Turn some things off
      set statusbar visible of container window to false
      set toolbar visible of container window to false

      # Size metrics { x, y, w, h }
      set the bounds of container window to {40, 40, 520, 520}

      # Abstract an object
      set theViewOptions to the icon view options of container window

      # Turn some more things off
      set arrangement of theViewOptions to not arranged

      # Set background image in HFS+ format referenced from image bundle
      set background picture of theViewOptions to file ".background:qupzilla-dmg-background.png"

      # Align the icons to the background mask
      set icon size of theViewOptions to 64

      set position of item "QupZilla" of container window to {55, 390}
      set position of item "Applications" of container window to {390, 390}

      # Since this is a dynamic template modifier script tell Finder not to do any registration of applications
      update without registering applications

      delay 5

    close
  end tell
end tell

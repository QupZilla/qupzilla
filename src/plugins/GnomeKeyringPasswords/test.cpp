#include <stdio.h>

extern "C" {
#include "gnome-keyring.h"
}

int main()
{
    guint32 itemId;
    GnomeKeyringAttributeList* attributes = gnome_keyring_attribute_list_new();

    gnome_keyring_attribute_list_append_string(attributes, "key", "value");

    GnomeKeyringResult result = gnome_keyring_item_create_sync(GNOME_KEYRING_DEFAULT,
                                GNOME_KEYRING_ITEM_GENERIC_SECRET,
                                "display-name",
                                attributes,
                                "passwrd-123",
                                TRUE, // Update if exists
                                &itemId);

    if (result == GNOME_KEYRING_RESULT_OK) {
        printf("OK, id=%d\n", itemId);
    }

    return 0;
}

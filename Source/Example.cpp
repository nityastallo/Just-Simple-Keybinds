#include <iostream>

#include "JSK/Keybinds.h"

int main()
{
    // Declare the keybind
    JSK::Keybind kb;

    // Declare recording settings
    JSK::RecordSettings st = { };

    // Set settings
    st.MaxKeys = 3;
    st.AnyKeyAccepts = true;
    st.ModifierKeysAllowed = true;
    st.MaxKeysIncludeModifierKeys = true;
    st.ModifierKeysAllowedWithABC = true;
    st.IncludeAcceptKeys = true;
    st.MustAccept = true;
    st.AcceptKeys = { VK_RETURN };

    // Apply the settings
    kb.SetRecordSettings ( st );
    
    std::printf ( "Recording...\n" );

    // Begin recording
    kb.Record ( );

    // Substitute for a proper message loop
    while ( true )
    {
        if ( !kb.IsRecording ( ) ) // Break once done
            break;

        // Update every iteration
        kb.Update ( );

        Sleep ( 20 );
    }

    std::printf ( "Recording complete.\n" );
    std::printf ( "Recorded keybind: %s\n", JSK::GetKeybindLabel(kb.GetKeys()).c_str() );
    std::printf ( "Listening for keybind...\n" );
    
    while ( true )
    {
        if ( kb.IsPressed ( ) ) // Print a message everytime the keybind is pressed
            std::printf ( "Keybind pressed.\n" );

        if ( kb.IsReleased ( ) ) // Print a message everytime the keybind is pressed
            std::printf ( "Keybind released.\n" );

        Sleep ( 20 );
    }
}
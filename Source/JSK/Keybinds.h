#ifndef __JSK__
#define __JSK__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <set>
#include <unordered_map>
#include <string>
#include <iosfwd>
#include <string.h>
#include <sstream>

namespace JSK
{
	// Alias for a set of keys.
	using Keys = std::set<BYTE>;

	// ASCII values for uppercase and lowercase alphabet ranges.
	constexpr BYTE ABC_UPPER_START = 65;
	constexpr BYTE ABC_UPPER_END = 90;
	constexpr BYTE ABC_LOWER_START = 97;
	constexpr BYTE ABC_LOWER_END = 122;

	// Bitmask values for detecting key states.
	constexpr BYTE HIGH_BIT_MASK = 0x80;
	constexpr BYTE MOUSE_ACTIVE_MASK = 0x100;

	namespace Key
	{
		// Set of modifier keys (e.g., Shift, Ctrl, Alt).
		Keys Modifier = {
			VK_SHIFT, VK_CONTROL, VK_MENU, VK_TAB, VK_CAPITAL, VK_NUMLOCK, VK_SCROLL
		};

		// Set of mouse keys (e.g., Left, Right, Middle buttons).
		Keys Mouse = {
			VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2
		};

		// Global blacklist. If you want to exclude keys from recording use 'RecordSettings.Blacklist'.
		Keys Blacklist = {};

		// Map of virtual key codes to their names for keys that cannot be retrieved using GetKeyNameTextA.
		std::unordered_map<BYTE, std::string> Names = {
			{ VK_LBUTTON, "LBM" },
			{ VK_MBUTTON, "MMB" },
			{ VK_RBUTTON, "RBM" },
			{ VK_XBUTTON1, "Backward" },
			{ VK_XBUTTON2, "Forward" },
			{ VK_NUMPAD0, "Num 0" },
			{ VK_NUMPAD1, "Num 1" },
			{ VK_NUMPAD2, "Num 2" },
			{ VK_NUMPAD3, "Num 3" },
			{ VK_NUMPAD4, "Num 4" },
			{ VK_NUMPAD5, "Num 5" },
			{ VK_NUMPAD6, "Num 6" },
			{ VK_NUMPAD7, "Num 7" },
			{ VK_NUMPAD8, "Num 8" },
			{ VK_NUMPAD9, "Num 9" },
			{ VK_ADD, "Num +" },
			{ VK_SUBTRACT, "Num -" },
			{ VK_MULTIPLY, "Num *" },
			{ VK_DIVIDE, "Num /" },
			{ VK_DECIMAL, "Num ." },
			{ VK_HOME, "Home" },
			{ VK_END, "End" },
			{ VK_UP, "Up Arrow" },
			{ VK_DOWN, "Down Arrow" },
			{ VK_LEFT, "Left Arrow" },
			{ VK_RIGHT, "Right Arrow" },
			{ VK_INSERT, "Insert" },
			{ VK_DELETE, "Delete" },
			{ VK_PRIOR, "Page Up" },
			{ VK_NEXT, "Page Down" },
			{ VK_PRINT, "Print" },
			{ VK_PAUSE, "Pause" },
			{ VK_NUMLOCK, "Numlock" },
		};

	} // namespace Key

	// Structure to store settings related to key recording.
	struct RecordSettings
	{
		BYTE MaxKeys; // Maximum number of keys allowed during recording.

		Keys Blacklist; // Set of keys excluded from recording.

		Keys AcceptKeys; // Keys that stop (and save) recording when pressed.

		bool IncludeAcceptKeys; // Whether to include 'AcceptKeys' in the recording.

		bool BlacklistIsWhitelist; // Treat 'Blacklist' as a whitelist instead.

		bool MouseKeysAllowed; // Whether mouse keys are allowed in recording.

		bool OnlyABC; // Restrict recording to alphabetic keys only.

		bool ModifierKeysAllowed; // Whether modifier keys are allowed in recording.

		bool ModifierKeysAllowedWithABC; // Allow modifiers with alphabetic keys.

		bool AnyKeyAccepts; // Stop recording on any key release.

		bool MaxKeysIncludeModifierKeys; // Include modifier keys in 'MaxKeys' limit.

		bool MustAccept; // Recording stops only when 'AcceptKeys' are pressed. Priority over 'AnyKeyAccepts'
	};

	// Checks whether a specific key is currently active.
	bool IsKeyActive ( BYTE Key )
	{
		return ( GetKeyState ( Key ) & HIGH_BIT_MASK ) != 0;
	}

	// Checks whether a specific mouse key is currently active.
	bool IsMouseActive ( BYTE Key )
	{
		return ( GetKeyState ( Key ) & MOUSE_ACTIVE_MASK ) != 0;
	}

	// Checks if a key is an alphabetical character.
	bool IsKeyABC ( BYTE Key )
	{
		return ( Key >= ABC_UPPER_START && Key <= ABC_UPPER_END ) ||
			( Key >= ABC_LOWER_START && Key <= ABC_LOWER_END );
	}

	// Checks if the key is a modifier key (e.g., Shift, Ctrl, Alt).
	bool IsModifierKey ( BYTE Key )
	{
		return Key::Modifier.count ( Key ) > 0;
	}

	// Checks if the key is a mouse key (e.g. Middle button).
	bool IsMouseKey ( BYTE Key )
	{
		return Key::Mouse.count ( Key ) > 0;
	}

	// Returns a set of all currently active keys.
	Keys GetActiveKeys ( void )
	{
		Keys Result;

		for ( BYTE Key = 1; Key < 255; Key++ )
		{
			if ( Key > 159 && Key < 166 )
				continue;

			if ( GetKeyState ( Key ) & 0x8000 )
				Result.insert ( Key );
		}

		return Result;
	}

	// Checks if all keys in a provided set are currently active.
	bool AreKeysActive ( Keys Keybind )
	{
		for ( const BYTE Key : Keybind )
			if ( !IsKeyActive ( Key ) )
				return false;

		return true;
	}

	// Converts a set of keys to a label. (e.g. { VK_RETURN, VK_F9 } -> "Enter + F9" )
	std::string GetKeybindLabel ( Keys Keybind )
	{
		if ( Keybind.size ( ) == 0 )
			return "None";

		std::stringstream label;
		bool fs = true;

		char Name[128];

		for ( BYTE Key : Keybind )
		{

			if ( Key::Names.count ( Key ) > 0 )
				strcpy_s ( Name, Key::Names.at ( Key ).c_str ( ) );
			else
				GetKeyNameTextA ( MapVirtualKeyA ( Key, MAPVK_VK_TO_VSC ) << 16, Name, sizeof ( Name ) );

			if ( !fs )
				label << " + ";

			label << Name;
			fs = false;
		}

		return label.str ( );
	}

	// Wrapper for a keybind with additional functionality
	class Keybind
	{
		public:
		Keybind ( void ) : m_record_settings ( { 2, {}, {VK_RETURN, VK_ESCAPE}, false, false, false, false, true, false, true, true, false } ) { } // Default settings
		~Keybind ( void ) noexcept { }

		Keybind ( const Keys &Keybind, const RecordSettings &Settings ) : m_keys ( Keybind ), m_record_settings ( Settings ) { }
		Keybind ( const Keys &Keybind ) : m_keys ( Keybind ) { }
		Keybind ( const RecordSettings &Settings ) : m_record_settings ( Settings ) { }

		// Sets the keybind to a new set of keys.
		void SetKeys ( Keys Keybind )
		{
			this->m_keys = Keybind;
		}

		// Checks if the keybind contains any keys
		bool IsEmpty ( void )
		{
			return this->m_keys.size ( ) == 0;
		}

		// Updates the recording settings.
		void SetRecordSettings ( const RecordSettings &Settings )
		{
			this->m_record_settings = Settings;
		}

		// Checks if this keybind is active.
		bool IsActive ( void )
		{
			for ( const BYTE Key : this->m_keys )
				if ( !( GetKeyState ( Key ) & HIGH_BIT_MASK ) )
					return false;

			return true;
		}

		// Same as IsActive(), but returns true only once per press cycle.
		bool IsPressed ( void )
		{
			bool isActive = this->IsActive ( );

			if ( !this->m_pressed && isActive )
			{
				this->m_pressed = true;

				return true;
			}

			if ( this->m_pressed && !isActive )
				this->m_pressed = false; // shhh... 

			return false;
		}

		// Same as IsPressed(), but returns true on release.
		bool IsReleased ( void )
		{
			bool isActive = this->IsActive ( );

			if ( !this->m_released && !isActive )
			{
				this->m_released = true;

				return true;
			}

			if ( this->m_released && isActive )
				this->m_released = false;

			return false;
		}

		// Retrieves the current set of keys in the keybind.
		Keys GetKeys ( void )
		{
			return this->m_keys;
		}

		// Checks if the keybind is currently recording.
		bool IsRecording ( void ) const
		{
			return this->m_recording;
		}

		// Starts recording with the current settings.
		void Record ( void )
		{
			if ( this->IsRecording ( ) )
				return;

			this->m_tm_record_keys.clear ( );
			this->m_recording = true;
		}

		// Stops the current recording session.
		void Stop ( void )
		{
			if ( !this->IsRecording ( ) )
				return;

			this->m_recording = false;
		}

		// Stops recording and saves the recorded keys.
		void Save ( void )
		{
			this->Stop ( );
			this->m_keys = this->m_tm_record_keys;
		}

		// Updates the recording session, capturing new active keys. Needs to be called every tick/frame.
		bool Update ( void )
		{
			if ( !this->IsRecording ( ) )
				return false;

			RecordSettings st = this->m_record_settings;

			size_t RecordedKeys = 0;

			for ( const BYTE Key : this->m_tm_record_keys )
			{
				if ( !st.MaxKeysIncludeModifierKeys && IsModifierKey ( Key ) )
					continue;

				RecordedKeys++;
			}

			if ( RecordedKeys >= st.MaxKeys )
			{
				this->Save ( );
				return true;
			}

			for ( BYTE i = 1; i < 255; i++ )
			{
				if ( i > 159 && i < 166 ) // Side specific modifier keys
					continue;

				bool isASCII = IsKeyABC ( i );
				bool isModifier = IsModifierKey ( i );
				bool isMouse = IsMouseKey ( i );
				bool isAccept = st.AcceptKeys.count ( i ) > 0;

				if ( isModifier && !st.ModifierKeysAllowed )
					continue;

				if ( isMouse && !st.MouseKeysAllowed )
					continue;

				if ( !isASCII )
					if ( isModifier && st.OnlyABC && !st.ModifierKeysAllowedWithABC )
						continue;

				if ( !isModifier && !isASCII && st.OnlyABC && !isAccept )
					continue;

				if ( isModifier && st.OnlyABC && !st.ModifierKeysAllowedWithABC && !isAccept )
					continue;

				bool inBlacklist = st.Blacklist.count ( i ) > 0;

				if ( ( inBlacklist && !st.BlacklistIsWhitelist ) || ( !inBlacklist && st.BlacklistIsWhitelist ) )
					continue;

				if ( Key::Blacklist.count ( i ) > 0 )
					continue;

				bool isActive = ( IsKeyActive ( i ) && !isMouse ) || ( IsMouseActive ( i ) && isMouse );
				bool isRecorded = this->m_tm_record_keys.count ( i ) > 0;

				if ( ( st.AcceptKeys.count ( i ) > 0 ) && isActive )
				{
					if ( st.IncludeAcceptKeys )
						this->m_tm_record_keys.insert ( i );

					this->Save ( );
					return true;
				}

				if ( isActive && !isRecorded )
				{
					this->m_tm_record_keys.insert ( i );
					continue;
				}

				if ( !isActive && isRecorded && !st.MustAccept )
				{
					this->Save ( );
					return true;
				}
			}

			return false;
		}

		// Retrieves currently active keys during recording.
		Keys GetRecordingPreview ( void )
		{
			if ( !this->IsRecording ( ) )
				return {};

			return this->m_tm_record_keys;
		}

		private:

		Keys m_keys = {}; // The keys
		bool m_recording = false; // Recording
		RecordSettings m_record_settings = {}; // Settings for recording

		// * * * //
		bool m_pressed = true;  // Auxiliary variable for IsActivated().
		bool m_released = true; // Auxiliary variable for IsDeactivated().

		Keys m_tm_record_keys = {}; // Active keys during recording
	};

} // namespace JSK

#endif // __JSK__

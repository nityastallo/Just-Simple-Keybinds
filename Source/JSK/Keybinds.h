#ifndef __JSK__
#define __JSK__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <algorithm>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <sstream>

namespace JSK
{
	using Keys = std::set<BYTE>;

	constexpr BYTE ABC_UPPER_START = 65;
	constexpr BYTE ABC_UPPER_END = 90;
	constexpr BYTE ABC_LOWER_START = 97;
	constexpr BYTE ABC_LOWER_END = 122;

	constexpr BYTE HIGH_BIT_MASK = 0x80;
	constexpr BYTE MOUSE_ACTIVE_MASK = 0x100;

	namespace Key
	{
		Keys Modifier = {
			VK_SHIFT, VK_CONTROL, VK_MENU, VK_TAB, VK_CAPITAL, VK_NUMLOCK, VK_SCROLL
		};

		Keys Mouse = {
			VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2
		};

		Keys Blacklist = {};

		// Some key names cannot be retrieved using GetKeyNameText and some pairs of keys aren't distinquished. 
		// We need to manually map these names.
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

	} // Key

	typedef struct tagRecordSettings
	{
		// Maximum number of keys that can be recorded. 
		// Once this limit is reached, the recording will automatically stop and save.
		BYTE MaxKeys;

		// A list of keys that are blacklisted from being recorded.
		Keys Blacklist;

		// A list of keys that, if pressed, will automatically stop recording and save the data.
		Keys AcceptKeys;

		// Determines whether the 'AcceptKeys' should be included in the recorded keybinds.
		// If true, 'AcceptKeys' will be counted as part of the key sequence when recorded.
		bool IncludeAcceptKeys;

		// If true, the 'Blacklist' is treated as a whitelist. Only keys listed in this list will be recorded.
		bool BlacklistIsWhitelist;

		// Specifies whether mouse keys should be allowed in the recording.
		bool MouseKeysAllowed;

		// If true, only alphabet characters will be recorded.
		bool OnlyABC;

		// If true, modifier keys (like Shift, Ctrl, Alt) are allowed to be recorded.
		bool ModifierKeysAllowed;

		// Determines whether modifier keys are allowed to be used in conjunction with alphabet keys.
		bool ModifierKeysAllowedWithABC;

		// If true, any key release will automatically stop the recording and save the data.
		bool AnyKeyAccepts;

		// If true, modifier keys will also count towards the 'MaxKeys' limit.
		bool MaxKeysIncludeModifierKeys;

		// If true, the recording will continue even after a key is released, 
		// and the recording will only stop when one of the 'AcceptKeys' is pressed.
		// Priority over 'AnyKeyAccepts'
		bool MustAccept;
	} RecordSettings;

	// Checks whether a specific key is currently pressed (active).
	bool IsKeyActive ( BYTE Key )
	{
		return ( GetKeyState ( Key ) & HIGH_BIT_MASK ) != 0;
	}

	// Checks whether a specific mouse key is currently pressed (active).
	bool IsMouseActive ( BYTE Key )
	{
		return ( GetKeyState ( Key ) & MOUSE_ACTIVE_MASK ) != 0;
	}

	// Checks if the key is an alphabetical character (either uppercase or lowercase).
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

	// Checks if all keys in a set are currently active.
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

	// Wrapper for the keys set with additional functionality
	class Keybind
	{
		public:
		Keybind ( void ) : m_record_settings( {2, {}, {VK_RETURN, VK_ESCAPE}, false, false, false, false, true, false, true, true, false} ) { } // Default settings
		Keybind ( const RecordSettings &Settings ) : m_record_settings(Settings) { }
		~Keybind ( void ) noexcept { }

		// Set settings for recording
		void SetRecordSettings ( const RecordSettings &Settings )
		{
			this->m_record_settings = Settings;
		}

		// Checks if all keys in this keybind are active.
		bool IsActive ( void )
		{
			for ( const BYTE Key : this->m_keys )
				if ( !( GetKeyState ( Key ) & HIGH_BIT_MASK ) )
					return false;

			return true;
		}

		// Checks if all keys are active just like 'IsActive()' but return true only once.
		bool IsPressed ( void )
		{
			bool isActive = this->IsActive();

			if ( !this->m_active && isActive )
			{
				this->m_active = true;

				return true;
			}

			return false;
		}

		// Same as 'IsPressed()' but returns true on release.
		bool IsReleased ( void )
		{
			bool isActive = this->IsActive ( );

			if ( this->m_active && !isActive )
			{
				this->m_active = false;

				return true;
			}

			return false;
		}

		// Returns the set of keys in the keybind.
		Keys GetKeys ( void )
		{
			return this->m_keys;
		}

		// Returns whether recording is currently active.
		bool IsRecording ( void )
		{
			return this->m_recording;
		}

		// Starts recording with the given settings (or default settings if none provided).
		void Record ( void )
		{
			if ( this->IsRecording ( ) )
				return;

			this->m_tm_record_keys.clear ( );
			this->m_recording = true;
		}

		// Stops the recording if it's currently in progress.
		void Stop ( void )
		{
			if ( !this->IsRecording ( ) )
				return;

			this->m_recording = false;
		}

		// Stops recording and saves the recorded keys.
		void Save ( void )
		{
			this->Stop();
			this->m_keys = this->m_tm_record_keys;
		}
	
		// Needs to be called every frame/tick. Scans for active keys while recording.
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
				if ( i > 159 && i < 166 )
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

				if ( (st.AcceptKeys.count(i) > 0) && isActive )
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

		// Returns active keys during recording for preview.
		Keys GetRecordingPreview ( void )
		{
			if ( !this->IsRecording ( ) )
				return {};

			return this->m_tm_record_keys;
		}

		private:

		Keys m_keys = {}; // The keys
		bool m_recording = false; // Recording
		RecordSettings m_record_settings = {}; // Settings used for recording

		// * * * //
		bool m_active = false; // Auxiliary variable for proper IsActivated() / IsDeactivated() functioning.
		Keys m_tm_record_keys = {}; // Active keys during recording
	};

} // JSK

#endif // __JSK__
# HADOC KeyBoard

A Windows Sinhala transliteration input method. Select **Sinhala - HADOC KeyBoard**
from the Windows input switcher, then type Singlish in any TSF-compatible editor.

- Switch input methods with **Win + Space** (alongside Default and Helakuru)
- The installer registers HADOC with the Windows Text Services Framework (TSF)
- Supports 64-bit and 32-bit Windows desktop applications
- No data is logged, stored or sent anywhere

## Typing guide

| Type | Get | Type | Get |
|------|-----|------|-----|
| `mama` | මම | `gedara` | ගෙදර |
| `yanawaa` | යනවා | `karanna` | කරන්න |
| `Srii` | ශ්‍රී | `prathama` | ප්‍රථම |
| `kya` | ක්‍ය | `sinhala` | සින්හල |

Vowels: `a අ` `aa ආ` `ae ඇ` `aae ඈ` `i ඉ` `ii ඊ` `u උ` `uu ඌ` `e එ` `ee ඒ` `o ඔ` `oo ඕ` `ai ඓ` `au ඖ`

Consonants: lowercase = common letter, uppercase / `h` = variants
`t ත` `th ථ` `T ට` `Th ඨ` · `d ද` `dh ධ` `D ඩ` `Dh ඪ` · `n න` `N ණ` `nd ඳ` `ng ඟ` `nng ඞ` ·
`l ල` `L ළ` · `s ස` `sh/S ශ` `Sh ෂ` · `k ක` `kh ඛ` · `g ග` `gh ඝ` · `c ච` `C ඡ` · `j ජ` `jh ඣ` ·
`p ප` `ph ඵ` · `b බ` `bh භ` `mb ඹ` · `m ම` `y ය` `r ර` `w ව` `h හ` `f ෆ` · `M ං` `H ඃ`

Consonant + `r` / `y` without a vowel gives rakaransaya / yansaya (`kra` ක්‍ර, `kya` ක්‍ය).

## Installation

Run the installer as administrator. It registers HADOC as a Sinhala (Sri Lanka)
TSF input method for both 64-bit and 32-bit applications.

After installation, press **Win + Space** and choose **Sinhala - HADOC KeyBoard**.
Choose Default or Helakuru from the same Windows input switcher to change back.

If Sinhala is not listed in Windows language settings, add **Sinhala (Sri Lanka)**
under Settings → Time & language → Language & region, then open the input switcher again.

## Build

Requires the .NET 8 SDK, Visual Studio C++ Build Tools, and Inno Setup 6.
The GitHub Actions workflow builds the tray controller and the x86/x64 TSF DLLs,
installs them in a clean runner, and checks their COM registrations before publishing.

## Compatibility

The TSF input processor works in TSF-compatible Windows apps. Some older or
non-TSF applications, games, secure desktops, and elevated applications may not
support third-party text services.

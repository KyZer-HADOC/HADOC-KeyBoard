# HADOC KeyBoard

A lightweight Sinhala typing app for Windows (Helakuru-style). Type Singlish
anywhere (Notepad, Word, browsers, chat apps) and get Sinhala Unicode.

- Toggle Sinhala / English: **Win+Space** (or **Ctrl+Space**, or click the tray icon)
- Lives in the system tray; no data is logged, stored or sent anywhere

## Typing guide

| Type | Get | Type | Get |
|------|-----|------|-----|
| \`mama\` | මම | \`gedara\` | ගෙදර |
| \`yanawaa\` | යනවා | \`karanna\` | කරන්න |
| \`Srii\` | ශ්‍රී | \`prathama\` | ප්‍රථම |
| \`kya\` | ක්‍ය | \`sinhala\` | සින්හල |

Vowels: \`a අ\` \`aa ආ\` \`ae ඇ\` \`aae ඈ\` \`i ඉ\` \`ii ඊ\` \`u උ\` \`uu ඌ\` \`e එ\` \`ee ඒ\` \`o ඔ\` \`oo ඕ\` \`ai ඓ\` \`au ඖ\`

Consonants: lowercase = common letter, uppercase / \`h\` = variants
\`t ත\` \`th ථ\` \`T ට\` \`Th ඨ\` · \`d ද\` \`dh ධ\` \`D ඩ\` \`Dh ඪ\` · \`n න\` \`N ණ\` \`nd ඳ\` \`ng ඟ\` \`nng ඞ\` ·
\`l ල\` \`L ළ\` · \`s ස\` \`sh/S ශ\` \`Sh ෂ\` · \`k ක\` \`kh ඛ\` · \`g ග\` \`gh ඝ\` · \`c ච\` \`C ඡ\` · \`j ජ\` \`jh ඣ\` ·
\`p ප\` \`ph ඵ\` · \`b බ\` \`bh භ\` \`mb ඹ\` · \`m ම\` \`y ය\` \`r ර\` \`w ව\` \`h හ\` \`f ෆ\` · \`M ං\` \`H ඃ\`

Consonant + \`r\` / \`y\` without a vowel gives rakaransaya / yansaya (\`kra\` ක්‍ර, \`kya\` ක්‍ය).
Edit the tables in \`HadocKeyboard/SinhalaEngine.cs\` to change the scheme.

## Build

Requires the .NET 8 SDK.

    dotnet run --project HadocKeyboard

### setup.exe

Pushing to GitHub builds it automatically (\`.github/workflows/build.yml\`):

- every push to \`main\`: download the installer and checksum from the workflow's **Artifacts**, and a versioned release is refreshed
- push a tag like \`v1.0.0\`: \`HADOC-KeyBoard-Setup.exe\` is attached to the **Releases** page

    git tag v1.0.0
    git push origin v1.0.0

To build locally on Windows: \`dotnet publish HadocKeyboard -c Release -r win-x64 --self-contained -p:PublishSingleFile=true -o publish\`,
then compile \`installer/setup.iss\` with Inno Setup (output: \`dist/HADOC-KeyBoard-Setup.exe\`).

## Known limitations

- Works by sending Backspace + Unicode text, so a few apps (some games, remote desktop,
  apps running as Administrator) may not behave. Run HADOC KeyBoard as admin for elevated windows.
- Antivirus / SmartScreen may warn about the unsigned installer; keyboard hooks are also
  used by keyloggers. The source code here is the full app.
- Caps Lock is ignored; use Shift for capital letters.

using System.Text;

namespace HadocKeyboard;

/// <summary>
/// Singlish (Latin letters) -> Sinhala Unicode.
/// Edit the tables below to change the typing scheme.
/// </summary>
public static class SinhalaEngine
{
    private const string Hal = "\u0DCA"; // ් (al-lakuna)
    private const string Zwj = "\u200D"; // joins rakaransaya / yansaya

    private static readonly Dictionary<string, string> Consonants = new()
    {
        ["k"] = "ක", ["kh"] = "ඛ", ["K"] = "ඛ",
        ["g"] = "ග", ["gh"] = "ඝ", ["G"] = "ඝ", ["ng"] = "ඟ", ["nng"] = "ඞ", ["gn"] = "ඥ",
        ["c"] = "ච", ["ch"] = "ච", ["C"] = "ඡ", ["chh"] = "ඡ",
        ["j"] = "ජ", ["jh"] = "ඣ", ["J"] = "ඣ", ["nj"] = "ඦ",
        ["t"] = "ත", ["th"] = "ථ", ["T"] = "ට", ["Th"] = "ඨ",
        ["d"] = "ද", ["dh"] = "ධ", ["D"] = "ඩ", ["Dh"] = "ඪ",
        ["n"] = "න", ["N"] = "ණ", ["nd"] = "ඳ", ["nD"] = "ඬ",
        ["p"] = "ප", ["ph"] = "ඵ", ["P"] = "ඵ",
        ["b"] = "බ", ["bh"] = "භ", ["B"] = "භ", ["mb"] = "ඹ", ["m"] = "ම",
        ["y"] = "ය", ["r"] = "ර", ["l"] = "ල", ["L"] = "ළ", ["w"] = "ව", ["v"] = "ව",
        ["s"] = "ස", ["sh"] = "ශ", ["S"] = "ශ", ["Sh"] = "ෂ",
        ["h"] = "හ", ["f"] = "ෆ",
    };

    // key -> (independent vowel, dependent sign)
    private static readonly Dictionary<string, (string Indep, string Sign)> Vowels = new()
    {
        ["aae"] = ("ඈ", "ෑ"), ["aa"] = ("ආ", "ා"), ["ae"] = ("ඇ", "ැ"), ["a"] = ("අ", ""),
        ["ii"] = ("ඊ", "ී"), ["i"] = ("ඉ", "ි"),
        ["uu"] = ("ඌ", "ූ"), ["u"] = ("උ", "ු"),
        ["ee"] = ("ඒ", "ේ"), ["e"] = ("එ", "ෙ"),
        ["oo"] = ("ඕ", "ෝ"), ["o"] = ("ඔ", "ො"),
        ["ai"] = ("ඓ", "ෛ"), ["au"] = ("ඖ", "ෞ"),
    };

    private static readonly Dictionary<char, string> Symbols = new()
    {
        ['M'] = "ං", ['H'] = "ඃ",
    };

    private static bool TryMatch<T>(Dictionary<string, T> table, string s, int i, out string key)
    {
        for (int len = Math.Min(3, s.Length - i); len >= 1; len--)
        {
            var k = s.Substring(i, len);
            if (table.ContainsKey(k)) { key = k; return true; }
        }
        key = "";
        return false;
    }

    /// <summary>Converts one Latin "word" (letters only) to Sinhala.</summary>
    public static string Convert(string s)
    {
        var sb = new StringBuilder();
        int i = 0;
        while (i < s.Length)
        {
            if (TryMatch(Consonants, s, i, out var ck))
            {
                i += ck.Length;
                if (TryMatch(Vowels, s, i, out var vk))
                {
                    sb.Append(Consonants[ck]).Append(Vowels[vk].Sign);
                    i += vk.Length;
                }
                else
                {
                    // consonant without a vowel: add al-lakuna.
                    // If r/y follows, join as rakaransaya / yansaya (ක්‍ර, ක්‍ය).
                    bool joins = i < s.Length && (s[i] == 'r' || s[i] == 'y');
                    sb.Append(Consonants[ck]).Append(Hal);
                    if (joins) sb.Append(Zwj);
                }
            }
            else if (TryMatch(Vowels, s, i, out var vk2))
            {
                sb.Append(Vowels[vk2].Indep);
                i += vk2.Length;
            }
            else
            {
                sb.Append(Symbols.TryGetValue(s[i], out var sym) ? sym : s[i].ToString());
                i++;
            }
        }
        return sb.ToString();
    }
}

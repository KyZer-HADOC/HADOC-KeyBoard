#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <msctf.h>
#include <oleauto.h>
#include <atomic>
#include <new>
#include <string>
#include <vector>
#include <algorithm>

static HMODULE g_module = nullptr;
static std::atomic<long> g_objects{0};
static std::atomic<long> g_locks{0};

static const CLSID CLSID_HadocKeyboardTip =
{ 0x97a2d4e1, 0x8b4f, 0x4e6a, { 0x9b, 0x31, 0x72, 0x6c, 0x18, 0xa4, 0xd2, 0x55 } };
static const GUID GUID_HadocKeyboardProfile =
{ 0x6c5b1f92, 0x2d0e, 0x43d9, { 0xa8, 0x11, 0x3f, 0x5e, 0x7a, 0x9c, 0x20, 0x41 } };

static constexpr LANGID HADOC_LANGID = MAKELANGID(0x5B, 0x01); // Sinhala (Sri Lanka)

static std::wstring ModulePath()
{
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(g_module, path, MAX_PATH);
    return path;
}

static bool IsModifierDown(int vk)
{
    return (GetKeyState(vk) & 0x8000) != 0;
}

static bool CtrlDown()
{
    return IsModifierDown(VK_CONTROL) || IsModifierDown(VK_LCONTROL) || IsModifierDown(VK_RCONTROL);
}
static bool AltDown()
{
    return IsModifierDown(VK_MENU) || IsModifierDown(VK_LMENU) || IsModifierDown(VK_RMENU);
}
static bool ShiftDown()
{
    return IsModifierDown(VK_SHIFT) || IsModifierDown(VK_LSHIFT) || IsModifierDown(VK_RSHIFT);
}
static bool WinDown()
{
    return IsModifierDown(VK_LWIN) || IsModifierDown(VK_RWIN);
}

using Pair = std::pair<std::wstring, std::wstring>;

static const std::vector<Pair>& Consonants()
{
    static const std::vector<Pair> values = {
        {L"nng",L"ඞ"}, {L"chh",L"ඡ"},
        {L"kh",L"ඛ"}, {L"gh",L"ඝ"}, {L"ng",L"ඟ"}, {L"gn",L"ඥ"},
        {L"ch",L"ච"}, {L"jh",L"ඣ"}, {L"nj",L"ඦ"},
        {L"th",L"ථ"}, {L"Th",L"ඨ"}, {L"dh",L"ධ"}, {L"Dh",L"ඪ"},
        {L"nd",L"ඳ"}, {L"nD",L"ඬ"}, {L"ph",L"ඵ"}, {L"bh",L"භ"},
        {L"mb",L"ඹ"}, {L"sh",L"ශ"}, {L"Sh",L"ෂ"},
        {L"K",L"ඛ"}, {L"G",L"ඝ"}, {L"C",L"ඡ"}, {L"J",L"ඣ"},
        {L"T",L"ට"}, {L"D",L"ඩ"}, {L"N",L"ණ"}, {L"P",L"ඵ"},
        {L"B",L"භ"}, {L"S",L"ශ"},
        {L"k",L"ක"}, {L"g",L"ග"}, {L"c",L"ච"}, {L"j",L"ජ"},
        {L"t",L"ත"}, {L"d",L"ද"}, {L"n",L"න"}, {L"p",L"ප"},
        {L"b",L"බ"}, {L"m",L"ම"}, {L"y",L"ය"}, {L"r",L"ර"},
        {L"l",L"ල"}, {L"L",L"ළ"}, {L"w",L"ව"}, {L"v",L"ව"},
        {L"s",L"ස"}, {L"h",L"හ"}, {L"f",L"ෆ"}
    };
    return values;
}

static const std::vector<Pair>& Vowels()
{
    static const std::vector<Pair> values = {
        {L"aae",L"ඈ"}, {L"aa",L"ආ"}, {L"ae",L"ඇ"},
        {L"ii",L"ඊ"}, {L"i",L"ඉ"}, {L"uu",L"ඌ"}, {L"u",L"උ"},
        {L"ee",L"ඒ"}, {L"e",L"එ"}, {L"oo",L"ඕ"}, {L"o",L"ඔ"},
        {L"ai",L"ඓ"}, {L"au",L"ඖ"}, {L"a",L"අ"}
    };
    return values;
}

static const std::vector<Pair>& VowelSigns()
{
    static const std::vector<Pair> values = {
        {L"aae",L"ෑ"}, {L"aa",L"ා"}, {L"ae",L"ැ"},
        {L"ii",L"ී"}, {L"i",L"ි"}, {L"uu",L"ූ"}, {L"u",L"ු"},
        {L"ee",L"ේ"}, {L"e",L"ෙ"}, {L"oo",L"ෝ"}, {L"o",L"ො"},
        {L"ai",L"ෛ"}, {L"au",L"ෞ"}, {L"a",L""}
    };
    return values;
}

static bool Match(const std::vector<Pair>& table, const std::wstring& s, size_t pos,
                  std::wstring& key, std::wstring& value)
{
    for (const auto& item : table)
    {
        if (pos + item.first.size() <= s.size() &&
            s.compare(pos, item.first.size(), item.first) == 0)
        {
            key = item.first;
            value = item.second;
            return true;
        }
    }
    return false;
}

static std::wstring ConvertSinhala(const std::wstring& input)
{
    const std::wstring hal = L"\x0DCA";
    const std::wstring zwj = L"\x200D";
    std::wstring output;
    size_t i = 0;

    while (i < input.size())
    {
        std::wstring key, value;
        if (Match(Consonants(), input, i, key, value))
        {
            i += key.size();
            std::wstring vowelKey, vowelValue;
            if (Match(Vowels(), input, i, vowelKey, vowelValue))
            {
                output += value;
                std::wstring signKey, signValue;
                if (Match(VowelSigns(), input, i, signKey, signValue))
                {
                    output += signValue;
                    i += signKey.size();
                }
                else
                {
                    output += vowelValue;
                    i += vowelKey.size();
                }
            }
            else
            {
                bool joins = i < input.size() && (input[i] == L'r' || input[i] == L'y');
                output += value;
                output += hal;
                if (joins) output += zwj;
            }
        }
        else if (Match(Vowels(), input, i, key, value))
        {
            output += value;
            i += key.size();
        }
        else
        {
            wchar_t ch = input[i++];
            if (ch == L'M') output += L"ං";
            else if (ch == L'H') output += L"ඃ";
            else output += ch;
        }
    }
    return output;
}

class ReplaceEditSession final : public ITfEditSession
{
public:
    ReplaceEditSession(ITfContext* context, const std::wstring& before,
                       const std::wstring& after, bool* caretValid)
        : m_refs(1), m_context(context), m_before(before), m_after(after), m_caretValid(caretValid)
    {
        if (m_context) m_context->AddRef();
        if (m_caretValid) *m_caretValid = true;
        ++g_objects;
    }

    ~ReplaceEditSession()
    {
        if (m_context) m_context->Release();
        --g_objects;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void** object) override
    {
        if (!object) return E_POINTER;
        *object = nullptr;
        if (iid == IID_IUnknown || iid == IID_ITfEditSession)
        {
            *object = static_cast<ITfEditSession*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override { return static_cast<ULONG>(++m_refs); }
    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG refs = static_cast<ULONG>(--m_refs);
        if (!refs) delete this;
        return refs;
    }

    STDMETHODIMP DoEditSession(TfEditCookie cookie) override
    {
        if (!m_context) return E_UNEXPECTED;

        TF_SELECTION selection{};
        ULONG fetched = 0;
        HRESULT hr = m_context->GetSelection(cookie, TF_DEFAULT_SELECTION, 1, &selection, &fetched);
        if (FAILED(hr) || fetched == 0 || !selection.range)
            return FAILED(hr) ? hr : E_FAIL;

        ITfRange* range = nullptr;
        hr = selection.range->Clone(&range);

        if (SUCCEEDED(hr) && !m_before.empty())
        {
            LONG comparison = 0;
            hr = selection.range->CompareStart(cookie, selection.range, TF_ANCHOR_END, &comparison);
            if (SUCCEEDED(hr) && comparison != 0)
            {
                if (m_caretValid) *m_caretValid = false;
                range->Release();
                selection.range->Release();
                return S_OK;
            }

            LONG shifted = 0;
            hr = range->ShiftStart(cookie, -static_cast<LONG>(m_before.size()), &shifted, nullptr);
            if (SUCCEEDED(hr) && shifted != -static_cast<LONG>(m_before.size()))
                hr = E_FAIL;

            if (SUCCEEDED(hr))
            {
                std::wstring existing(m_before.size() + 1, L'\0');
                ULONG actual = 0;
                hr = range->GetText(cookie, 0, existing.data(),
                                    static_cast<ULONG>(existing.size()), &actual);
                if (SUCCEEDED(hr))
                {
                    existing.resize(static_cast<size_t>(actual));
                    if (existing != m_before)
                    {
                        if (m_caretValid) *m_caretValid = false;
                        range->Release();
                        selection.range->Release();
                        return S_OK;
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
            hr = range->SetText(cookie, 0, m_after.c_str(), static_cast<LONG>(m_after.size()));

        if (SUCCEEDED(hr))
            hr = range->Collapse(cookie, TF_ANCHOR_END);

        if (SUCCEEDED(hr))
        {
            TF_SELECTION newSelection{};
            newSelection.range = range;
            newSelection.style = TF_AE_END;
            hr = m_context->SetSelection(cookie, 1, &newSelection);
        }

        if (range) range->Release();
        selection.range->Release();
        return hr;
    }

private:
    std::atomic<long> m_refs;
    ITfContext* m_context;
    std::wstring m_before;
    std::wstring m_after;
    bool* m_caretValid;
};

class HadocTextService final : public ITfTextInputProcessor, public ITfKeyEventSink
{
public:
    HadocTextService() : m_refs(1), m_threadMgr(nullptr), m_clientId(TF_CLIENTID_NULL),
                         m_lastContext(nullptr)
    {
        ++g_objects;
    }

    ~HadocTextService()
    {
        ResetBuffer();
        if (m_lastContext) m_lastContext->Release();
        if (m_threadMgr) m_threadMgr->Release();
        --g_objects;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void** object) override
    {
        if (!object) return E_POINTER;
        *object = nullptr;
        if (iid == IID_IUnknown || iid == IID_ITfTextInputProcessor)
            *object = static_cast<ITfTextInputProcessor*>(this);
        else if (iid == IID_ITfKeyEventSink)
            *object = static_cast<ITfKeyEventSink*>(this);
        else
            return E_NOINTERFACE;
        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() override { return static_cast<ULONG>(++m_refs); }
    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG refs = static_cast<ULONG>(--m_refs);
        if (!refs) delete this;
        return refs;
    }

    STDMETHODIMP Activate(ITfThreadMgr* threadMgr, TfClientId clientId) override
    {
        if (!threadMgr) return E_INVALIDARG;
        m_threadMgr = threadMgr;
        m_threadMgr->AddRef();
        m_clientId = clientId;

        ITfKeystrokeMgr* keyMgr = nullptr;
        HRESULT hr = m_threadMgr->QueryInterface(IID_ITfKeystrokeMgr, reinterpret_cast<void**>(&keyMgr));
        if (SUCCEEDED(hr) && keyMgr)
        {
            hr = keyMgr->AdviseKeyEventSink(m_clientId, this, TRUE);
            keyMgr->Release();
        }
        if (FAILED(hr))
        {
            m_threadMgr->Release();
            m_threadMgr = nullptr;
            m_clientId = TF_CLIENTID_NULL;
        }
        return hr;
    }

    STDMETHODIMP Deactivate() override
    {
        if (m_threadMgr)
        {
            ITfKeystrokeMgr* keyMgr = nullptr;
            if (SUCCEEDED(m_threadMgr->QueryInterface(IID_ITfKeystrokeMgr, reinterpret_cast<void**>(&keyMgr))) && keyMgr)
            {
                keyMgr->UnadviseKeyEventSink(m_clientId);
                keyMgr->Release();
            }
            m_threadMgr->Release();
            m_threadMgr = nullptr;
        }
        m_clientId = TF_CLIENTID_NULL;
        if (m_lastContext) { m_lastContext->Release(); m_lastContext = nullptr; }
        ResetBuffer();
        return S_OK;
    }

    STDMETHODIMP OnSetFocus(BOOL) override { ResetBuffer(); return S_OK; }

    STDMETHODIMP OnTestKeyDown(ITfContext*, WPARAM key, LPARAM, BOOL* eaten) override
    {
        if (!eaten) return E_INVALIDARG;
        *eaten = FALSE;

        const int vk = static_cast<int>(key);
        if (CtrlDown() || AltDown() || WinDown())
        {
            ResetBuffer();
            return S_OK;
        }

        if (vk >= 'A' && vk <= 'Z')
        {
            *eaten = TRUE;
            return S_OK;
        }
        if (vk == VK_BACK && !m_buffer.empty())
        {
            *eaten = TRUE;
            return S_OK;
        }

        // Shift is a modifier, not a word separator. Other unhandled keys
        // (space, punctuation, arrows, etc.) finish the current transliteration.
        if (vk != VK_SHIFT && vk != VK_LSHIFT && vk != VK_RSHIFT)
            ResetBuffer();
        return S_OK;
    }

    STDMETHODIMP OnTestKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) override
    {
        if (!eaten) return E_INVALIDARG;
        *eaten = FALSE;
        return S_OK;
    }

    STDMETHODIMP OnKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) override
    {
        if (!eaten) return E_INVALIDARG;
        *eaten = FALSE;
        return S_OK;
    }

    STDMETHODIMP OnPreservedKey(ITfContext*, REFGUID, BOOL* eaten) override
    {
        if (!eaten) return E_INVALIDARG;
        *eaten = FALSE;
        return S_OK;
    }

    STDMETHODIMP OnKeyDown(ITfContext* context, WPARAM key, LPARAM, BOOL* eaten) override
    {
        if (!eaten || !context) return E_INVALIDARG;
        *eaten = FALSE;

        const int vk = static_cast<int>(key);
        if (CtrlDown() || AltDown() || WinDown())
        {
            ResetBuffer();
            return S_OK;
        }

        if (vk >= 'A' && vk <= 'Z')
        {
            if (m_lastContext != context)
            {
                if (m_lastContext) m_lastContext->Release();
                m_lastContext = context;
                m_lastContext->AddRef();
                ResetBuffer();
            }

            wchar_t letter = static_cast<wchar_t>(ShiftDown() ? vk : (vk + 32));
            std::wstring nextBuffer = m_buffer + letter;
            std::wstring nextText = ConvertSinhala(nextBuffer);

            bool caretValid = true;
            HRESULT hr = ReplaceText(context, m_emitted, nextText, &caretValid);
            if (FAILED(hr)) return S_OK;

            if (!caretValid && !m_emitted.empty())
            {
                // The caret was moved or the text was edited elsewhere. Start
                // a fresh word at the current caret instead of deleting old text.
                nextBuffer.assign(1, letter);
                nextText = ConvertSinhala(nextBuffer);
                hr = ReplaceText(context, L"", nextText, &caretValid);
                if (FAILED(hr) || !caretValid) return S_OK;
            }

            m_buffer = nextBuffer;
            m_emitted = nextText;
            *eaten = TRUE;
            return S_OK;
        }

        if (vk == VK_BACK && !m_buffer.empty())
        {
            std::wstring nextBuffer = m_buffer.substr(0, m_buffer.size() - 1);
            std::wstring nextText = ConvertSinhala(nextBuffer);
            bool caretValid = true;
            HRESULT hr = ReplaceText(context, m_emitted, nextText, &caretValid);
            if (SUCCEEDED(hr) && caretValid)
            {
                m_buffer = nextBuffer;
                m_emitted = nextText;
                *eaten = TRUE;
            }
            else
            {
                ResetBuffer();
            }
            return S_OK;
        }

        if (vk != VK_SHIFT && vk != VK_LSHIFT && vk != VK_RSHIFT)
            ResetBuffer();
        return S_OK;
    }

private:
    HRESULT ReplaceText(ITfContext* context, const std::wstring& before,
                        const std::wstring& after, bool* caretValid)
    {
        ReplaceEditSession* session = new (std::nothrow) ReplaceEditSession(context, before, after, caretValid);
        if (!session) return E_OUTOFMEMORY;

        HRESULT sessionResult = E_FAIL;
        HRESULT hr = context->RequestEditSession(m_clientId, session,
                                                  TF_ES_SYNC | TF_ES_READWRITE,
                                                  &sessionResult);
        session->Release();
        if (FAILED(hr)) return hr;
        return sessionResult;
    }

    void ResetBuffer()
    {
        m_buffer.clear();
        m_emitted.clear();
    }

    std::atomic<long> m_refs;
    ITfThreadMgr* m_threadMgr;
    TfClientId m_clientId;
    ITfContext* m_lastContext;
    std::wstring m_buffer;
    std::wstring m_emitted;
};

class HadocClassFactory final : public IClassFactory
{
public:
    HadocClassFactory() : m_refs(1) { ++g_objects; }
    ~HadocClassFactory() { --g_objects; }

    STDMETHODIMP QueryInterface(REFIID iid, void** object) override
    {
        if (!object) return E_POINTER;
        *object = nullptr;
        if (iid == IID_IUnknown || iid == IID_IClassFactory)
        {
            *object = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override { return static_cast<ULONG>(++m_refs); }
    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG refs = static_cast<ULONG>(--m_refs);
        if (!refs) delete this;
        return refs;
    }

    STDMETHODIMP CreateInstance(IUnknown* outer, REFIID iid, void** object) override
    {
        if (outer) return CLASS_E_NOAGGREGATION;
        HadocTextService* service = new (std::nothrow) HadocTextService();
        if (!service) return E_OUTOFMEMORY;
        HRESULT hr = service->QueryInterface(iid, object);
        service->Release();
        return hr;
    }

    STDMETHODIMP LockServer(BOOL lock) override
    {
        if (lock) ++g_locks;
        else --g_locks;
        return S_OK;
    }

private:
    std::atomic<long> m_refs;
};

static HRESULT SetRegistryString(HKEY root, const std::wstring& subkey,
                                 const wchar_t* valueName, const std::wstring& value)
{
    HKEY key = nullptr;
    LONG status = RegCreateKeyExW(root, subkey.c_str(), 0, nullptr, 0,
                                  KEY_WRITE, nullptr, &key, nullptr);
    if (status != ERROR_SUCCESS) return HRESULT_FROM_WIN32(status);

    status = RegSetValueExW(key, valueName, 0, REG_SZ,
                            reinterpret_cast<const BYTE*>(value.c_str()),
                            static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(key);
    return HRESULT_FROM_WIN32(status);
}

static std::wstring ClsidString()
{
    wchar_t text[64]{};
    StringFromGUID2(CLSID_HadocKeyboardTip, text, 64);
    return text;
}

static HRESULT RegisterTsfProfile()
{
    ITfInputProcessorProfiles* profiles = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr,
                                  CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles,
                                  reinterpret_cast<void**>(&profiles));
    if (FAILED(hr) || !profiles) return hr;

    HRESULT registerHr = profiles->Register(CLSID_HadocKeyboardTip);
    BOOL enabled = FALSE;
    HRESULT checkHr = profiles->IsEnabledLanguageProfile(
        CLSID_HadocKeyboardTip, HADOC_LANGID, GUID_HadocKeyboardProfile, &enabled);

    if (SUCCEEDED(checkHr) && enabled)
    {
        registerHr = S_OK; // Already registered by the other DLL bitness.
    }
    else
    {
        const std::wstring iconPath = ModulePath();
        hr = profiles->AddLanguageProfile(
            CLSID_HadocKeyboardTip, HADOC_LANGID, GUID_HadocKeyboardProfile,
            L"HADOC KeyBoard", static_cast<ULONG>(-1),
            iconPath.c_str(), static_cast<ULONG>(-1), 0);
        if (hr == TF_E_ALREADY_EXISTS)
            hr = profiles->EnableLanguageProfile(
                CLSID_HadocKeyboardTip, HADOC_LANGID, GUID_HadocKeyboardProfile, TRUE);
        else if (SUCCEEDED(hr))
            hr = profiles->EnableLanguageProfile(
                CLSID_HadocKeyboardTip, HADOC_LANGID, GUID_HadocKeyboardProfile, TRUE);

        if (FAILED(hr)) registerHr = hr;
        else registerHr = S_OK;
    }

    ITfCategoryMgr* categories = nullptr;
    HRESULT categoryHr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr,
                                          CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                          reinterpret_cast<void**>(&categories));
    if (SUCCEEDED(categoryHr) && categories)
    {
        categoryHr = categories->RegisterCategory(
            CLSID_HadocKeyboardTip, GUID_TFCAT_TIP_KEYBOARD, CLSID_HadocKeyboardTip);
        categories->Release();
    }

    profiles->Release();
    if (FAILED(registerHr)) return registerHr;
    if (FAILED(categoryHr) && categoryHr != TF_E_ALREADY_EXISTS) return categoryHr;
    return S_OK;
}

static void UnregisterTsfProfile()
{
    ITfInputProcessorProfiles* profiles = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr,
                                   CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles,
                                   reinterpret_cast<void**>(&profiles))) && profiles)
    {
        profiles->RemoveLanguageProfile(CLSID_HadocKeyboardTip, HADOC_LANGID, GUID_HadocKeyboardProfile);
        profiles->Unregister(CLSID_HadocKeyboardTip);
        profiles->Release();
    }

    ITfCategoryMgr* categories = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, nullptr,
                                   CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                   reinterpret_cast<void**>(&categories))) && categories)
    {
        categories->UnregisterCategory(
            CLSID_HadocKeyboardTip, GUID_TFCAT_TIP_KEYBOARD, CLSID_HadocKeyboardTip);
        categories->Release();
    }
}

#ifdef _M_IX86
#pragma comment(linker, "/EXPORT:DllGetClassObject=_DllGetClassObject@12")
#pragma comment(linker, "/EXPORT:DllCanUnloadNow=_DllCanUnloadNow@0")
#pragma comment(linker, "/EXPORT:DllRegisterServer=_DllRegisterServer@0")
#pragma comment(linker, "/EXPORT:DllUnregisterServer=_DllUnregisterServer@0")
#else
#pragma comment(linker, "/EXPORT:DllGetClassObject")
#pragma comment(linker, "/EXPORT:DllCanUnloadNow")
#pragma comment(linker, "/EXPORT:DllRegisterServer")
#pragma comment(linker, "/EXPORT:DllUnregisterServer")
#endif

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID iid, void** object)
{
    if (clsid != CLSID_HadocKeyboardTip) return CLASS_E_CLASSNOTAVAILABLE;
    if (!object) return E_POINTER;
    *object = nullptr;

    HadocClassFactory* factory = new (std::nothrow) HadocClassFactory();
    if (!factory) return E_OUTOFMEMORY;
    HRESULT hr = factory->QueryInterface(iid, object);
    factory->Release();
    return hr;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return (g_objects.load() == 0 && g_locks.load() == 0) ? S_OK : S_FALSE;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
    const std::wstring clsid = ClsidString();
    const std::wstring base = L"CLSID\\" + clsid;
    const std::wstring modulePath = ModulePath();

    HRESULT hr = SetRegistryString(HKEY_CLASSES_ROOT, base, nullptr, L"HADOC KeyBoard Text Service");
    if (SUCCEEDED(hr))
        hr = SetRegistryString(HKEY_CLASSES_ROOT, base + L"\\InprocServer32", nullptr, modulePath);
    if (SUCCEEDED(hr))
        hr = SetRegistryString(HKEY_CLASSES_ROOT, base + L"\\InprocServer32", L"ThreadingModel", L"Apartment");
    if (FAILED(hr)) return hr;

    const HRESULT initHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool uninitialize = (initHr == S_OK || initHr == S_FALSE);
    if (FAILED(initHr) && initHr != RPC_E_CHANGED_MODE) return initHr;

    hr = RegisterTsfProfile();
    if (uninitialize) CoUninitialize();
    if (FAILED(hr))
    {
        RegDeleteTreeW(HKEY_CLASSES_ROOT, base.c_str());
        return hr;
    }
    return S_OK;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    const HRESULT initHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool uninitialize = (initHr == S_OK || initHr == S_FALSE);
    if (SUCCEEDED(initHr) || initHr == RPC_E_CHANGED_MODE)
        UnregisterTsfProfile();
    if (uninitialize) CoUninitialize();

    const std::wstring base = L"CLSID\\" + ClsidString();
    LONG status = RegDeleteTreeW(HKEY_CLASSES_ROOT, base.c_str());
    return (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND)
        ? S_OK : HRESULT_FROM_WIN32(status);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_module = instance;
        DisableThreadLibraryCalls(instance);
    }
    return TRUE;
}

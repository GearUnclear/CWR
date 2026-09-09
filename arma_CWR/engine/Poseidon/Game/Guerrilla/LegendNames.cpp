#include <Poseidon/Game/Guerrilla/LegendNames.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp> // stricmp

#include <string.h>

namespace Poseidon::Guerrilla
{

namespace
{

// The tables below are GENERATED. Regenerate with
//   python tools/legend-names/gen_legend_names.py --write
// and verify a hand edit has not drifted from the issue #57 source with
//   python tools/legend-names/gen_legend_names.py --check
// The source of truth is tests/fixtures/legend-names/issue57-names.json,
// committed so the transcription can still be re-verified after the session
// that made it. Strings are verbatim: apostrophes, hyphens and interior
// spaces are part of the name, nothing is deduplicated across pools (Haddad
// legitimately appears in three) and nothing is reordered.

// BEGIN GENERATED NAME TABLES
static const char* const kFirst_west_africa[] = {"Adebayo", "Kwame",  "Chinedu", "Kofi",    "Sekou",
                                                 "Amadou",  "Moussa", "Ibrahim", "Ousmane", "Babacar"};
static const char* const kLast_west_africa[] = {"Okafor", "Mensah", "Eze",    "Boateng", "Diallo",
                                                "Traore", "Keita",  "Camara", "Diop",    "Toure"};

static const char* const kFirst_east_africa[] = {"Jabari", "Dawit", "Bereket", "Yonatan", "Abdi",
                                                 "Tomas",  "Kamau", "Baraka",  "Tesfaye", "Ismail"};
static const char* const kLast_east_africa[] = {"Mwangi",  "Tesfaye", "Gebremariam", "Bekele", "Warsame",
                                                "Njoroge", "Odinga",  "Kiptoo",      "Abebe",  "Nur"};

static const char* const kFirst_southern_africa[] = {"Thabo",   "Tendai",   "Lwazi",  "Sipho",  "Bongani",
                                                     "Tawanda", "Sibusiso", "Themba", "Kagiso", "Mandla"};
static const char* const kLast_southern_africa[] = {"Mokoena", "Ndlovu", "Dlamini", "Khumalo", "Mthembu",
                                                    "Moyo",    "Zulu",   "Nkosi",   "Molefe",  "Sithole"};

static const char* const kFirst_north_africa[] = {"Yassine", "Rachid", "Amine",  "Walid", "Mehdi",
                                                  "Karim",   "Nabil",  "Hicham", "Adel",  "Samir"};
static const char* const kLast_north_africa[] = {"El Mansouri", "Bennani", "Cherif", "Bouzid", "Khelifi",
                                                 "Belkacem",    "Bensaid", "Amrani", "Mekki",  "Trabelsi"};

static const char* const kFirst_levant[] = {"Laith", "Ziad", "Tamer", "Fadi",    "Basil", "Marwan",
                                            "Nadim", "Jad",  "Wael",  "Ghassan", "Rami",  "Faris"};
static const char* const kLast_levant[] = {"Haddad", "Khoury", "Nassar",  "Mansour", "Halabi", "Khalil",
                                           "Saad",   "Hariri", "Darwish", "Hamdan",  "Najjar", "Saleh"};

static const char* const kFirst_palestine[] = {"Yazan", "Laith", "Tareq",   "Bashar", "Iyad",   "Fadi",  "Rami",
                                               "Nidal", "Ammar", "Mahmoud", "Samer",  "Wael",   "Anas",  "Khalil",
                                               "Ziad",  "Majd",  "Qais",    "Adnan",  "Nasser", "Marwan"};
static const char* const kLast_palestine[] = {
    "Barghouti", "Khalidi", "Nusseibeh", "Tamimi",    "Husseini", "Qudwa",  "Shawa",   "Masri",   "Khatib", "Darwish",
    "Najjar",    "Hamdan",  "Sabbagh",   "Abu-Salim", "Jarrar",   "Kanaan", "Shaheen", "Salameh", "Haddad", "Mansour"};

static const char* const kFirst_lebanon[] = {"Jad",     "Nadim",  "Ziad",  "Fadi",  "Rami", "Charbel", "Marwan",
                                             "Ghassan", "Bassem", "Tarek", "Elias", "Tony", "Karim",   "Wael",
                                             "Georges", "Nabil",  "Rabih", "Walid", "Sami", "Michel"};
static const char* const kLast_lebanon[] = {"Khoury", "Haddad", "Saad",   "Hariri",  "Gemayel", "Chamoun", "Maalouf",
                                            "Saliba", "Nassar", "Karam",  "Hobeika", "Sfeir",   "Doumit",  "Aoun",
                                            "Farah",  "Abboud", "Najjar", "Mansour", "Azar",    "Dagher"};

static const char* const kFirst_ireland[] = {"Cian",   "Ronan", "Declan",  "Conor",   "Eoin",   "Oisin",  "Niall",
                                             "Ciaran", "Fionn", "Darragh", "Padraig", "Seamus", "Cathal", "Diarmuid",
                                             "Colm",   "Tadhg", "Aidan",   "Brendan", "Cormac", "Lorcan"};
static const char* const kLast_ireland[] = {
    "Murphy",    "Kelly",   "O'Sullivan", "Walsh",  "Byrne", "Ryan",    "O'Connor", "O'Neill",    "Doyle",  "McCarthy",
    "Gallagher", "Kennedy", "Lynch",      "Murray", "Quinn", "Doherty", "Brennan",  "Fitzgerald", "Molloy", "Kavanagh"};

static const char* const kFirst_iraq[] = {"Haydar", "Mustafa", "Haider",    "Qasim",   "Yasin",
                                          "Ali",    "Sajjad",  "Muntadhar", "Hussein", "Karrar"};
static const char* const kLast_iraq[] = {"Al-Khafaji",  "Al-Tikriti", "Al-Janabi",   "Al-Rawi",    "Al-Hadithi",
                                         "Al-Samarrai", "Al-Bayati",  "Al-Moussawi", "Al-Dulaimi", "Al-Tamimi"};

static const char* const kFirst_arabian_peninsula[] = {"Fahad", "Saif",  "Sultan", "Rashid", "Nasser",
                                                       "Majid", "Hamad", "Mishal", "Ammar",  "Muadh"};
static const char* const kLast_arabian_peninsula[] = {"Al-Qahtani", "Al-Nuaimi", "Al-Mansoori", "Al-Harbi", "Al-Dosari",
                                                      "Al-Marri",   "Al-Hakimi", "Al-Sabri",    "Al-Yafai", "Al-Qadhi"};

static const char* const kFirst_iran[] = {"Arash",   "Kaveh",  "Navid",  "Pouya",  "Farhad",
                                          "Dariush", "Shahin", "Behzad", "Kamran", "Omid"};
static const char* const kLast_iran[] = {"Farhadi", "Rahimi", "Jafari",  "Karimi",   "Mehrabi",
                                         "Azadi",   "Nouri",  "Rostami", "Ebrahimi", "Kazemi"};

static const char* const kFirst_kurdish[] = {"Baran",  "Serhat", "Azad",    "Kawa", "Soran",
                                             "Shivan", "Rebin",  "Dilshad", "Aram", "Sherko"};
static const char* const kLast_kurdish[] = {"Karaman", "Hassan", "Mahmoud", "Rashid",  "Barzani",
                                            "Karimi",  "Azizi",  "Saeed",   "Hawrami", "Salih"};

static const char* const kFirst_turkey[] = {"Emre", "Kerem", "Mert",  "Burak", "Eren",
                                            "Cem",  "Ozan",  "Tolga", "Arda",  "Kaan"};
static const char* const kLast_turkey[] = {"Yilmaz", "Aydin", "Demir", "Kaya",  "Arslan",
                                           "Aksoy",  "Koc",   "Celik", "Sahin", "Kurt"};

static const char* const kFirst_south_asia[] = {"Arjun",  "Vikram", "Ravi",   "Dev",    "Kiran",
                                                "Ishaan", "Rohan",  "Aditya", "Naveen", "Imran"};
static const char* const kLast_south_asia[] = {"Mehta",  "Rao",   "Patel",   "Malhotra", "Sharma",
                                               "Kapoor", "Singh", "Chandra", "Perera",   "Qureshi"};

static const char* const kFirst_himalayan[] = {"Prabhat", "Tenzin", "Sonam",  "Pemba", "Karma",
                                               "Nima",    "Dorje",  "Pasang", "Jigme", "Tshering"};
static const char* const kLast_himalayan[] = {"Gurung", "Norbu",  "Wangchuk", "Sherpa", "Dorji",
                                              "Lama",   "Tamang", "Rai",      "Thapa",  "Bhutia"};

static const char* const kFirst_china[] = {"Wei", "Jun", "Hao", "Tao", "Jian", "Lei", "Bo", "Ming", "Peng", "Cheng"};
static const char* const kLast_china[] = {"Zhang", "Chen", "Liu", "Wang", "Li", "Zhao", "Huang", "Wu", "Xu", "Sun"};

static const char* const kFirst_korea[] = {"Minho",  "Joon",     "Jihoon", "Taeyang", "Hyunwoo",
                                           "Seojun", "Donghyun", "Jaemin", "Junho",   "Sungmin"};
static const char* const kLast_korea[] = {"Kim", "Park", "Lee", "Choi", "Jung", "Kang", "Cho", "Yoon", "Lim", "Han"};

static const char* const kFirst_japan[] = {"Haruto", "Ren",    "Daichi", "Kenji", "Kaito",
                                           "Takumi", "Hiroto", "Riku",   "Sota",  "Akira"};
static const char* const kLast_japan[] = {"Nakamura", "Takahashi", "Sato",      "Mori", "Tanaka",
                                          "Ito",      "Yamamoto",  "Kobayashi", "Kato", "Watanabe"};

static const char* const kFirst_mainland_southeast_asia[] = {"Minh",  "Quang", "Somchai", "Niran",     "Aung",
                                                             "Thura", "Sokha", "Vannak",  "Khampheng", "Chanthou"};
static const char* const kLast_mainland_southeast_asia[] = {"Nguyen", "Tran",  "Chaiyaporn", "Sukhum",     "Kyaw",
                                                            "Min",    "Chhay", "Sok",        "Phommasone", "Seng"};

static const char* const kFirst_maritime_southeast_asia[] = {"Arif",  "Bima",  "Rizal", "Agus",   "Dimas",
                                                             "Fajar", "Rizky", "Bagus", "Bayani", "Isagani"};
static const char* const kLast_maritime_southeast_asia[] = {"Santoso", "Pranoto", "Hakim",   "Setiawan", "Pratama",
                                                            "Hidayat", "Saputra", "Nugroho", "Santos",   "Reyes"};

static const char* const kFirst_central_asia[] = {"Temur",   "Bekzod", "Nursultan", "Azamat", "Rustam",
                                                  "Dilshod", "Eldar",  "Alisher",   "Timur",  "Bakhrom"};
static const char* const kLast_central_asia[] = {"Karimov",  "Rahimov", "Akhmetov", "Sadykov", "Iskandarov",
                                                 "Tursunov", "Nazarov", "Usmanov",  "Yusupov", "Mirzaev"};

static const char* const kFirst_mongolia[] = {"Baatar",    "Altan",   "Batsaikhan", "Temuulen", "Ankhbayar",
                                              "Erdenebat", "Chuluun", "Ganbaatar",  "Munkh",    "Batbold"};
static const char* const kLast_mongolia[] = {"Erdene",     "Batbayar", "Ganbold",   "Bat-Erdene", "Munkhbat",
                                             "Boldbaatar", "Dorj",     "Enkhbayar", "Bataa",      "Sukhbaatar"};

static const char* const kFirst_caucasus[] = {"Levan",  "Aram",  "Vahan", "Giorgi", "Nika",
                                              "Rashad", "Tural", "Davit", "Sandro", "Hayk"};
static const char* const kLast_caucasus[] = {"Beridze",  "Sarkisyan", "Petrosyan",  "Kapanadze", "Tsereteli",
                                             "Mammadov", "Aliyev",    "Gelashvili", "Hakobyan",  "Aslanov"};

static const char* const kFirst_balkans[] = {"Milan",  "Nikola",     "Luka",  "Dragan", "Mihai",
                                             "Stefan", "Aleksandar", "Bojan", "Marko",  "Andrei"};
static const char* const kLast_balkans[] = {"Jovanovic", "Markovic", "Horvat",    "Stojanovic", "Popescu",
                                            "Petrovic",  "Ilic",     "Kovacevic", "Marin",      "Ionescu"};

static const char* const kFirst_eastern_europe[] = {"Dimitri", "Marek", "Tomasz", "Oleksiy", "Mykola",
                                                    "Ilya",    "Pavel", "Maksim", "Bohdan",  "Yaroslav"};
static const char* const kLast_eastern_europe[] = {"Petrov",  "Kowalski", "Nowak",   "Koval",      "Bondarenko",
                                                   "Morozov", "Sokolov",  "Lebedev", "Shevchenko", "Melnyk"};

static const char* const kFirst_latin_america[] = {"Diego",    "Mateo",   "Thiago", "Caio",  "Rafael",
                                                   "Emiliano", "Joaquin", "Inti",   "Amaru", "Santiago"};
static const char* const kLast_latin_america[] = {"Quispe", "Huaman",  "Nascimento", "Ferreira", "Mendoza",
                                                  "Rojas",  "Salazar", "Condori",    "Mamani",   "Castillo"};

static const char* const kFirst_caribbean[] = {"Jean-Baptiste", "Wyclef",  "Andre",  "Kemar",   "Dario",
                                               "Marlon",        "Desmond", "Javier", "Renaldo", "Kofi"};
static const char* const kLast_caribbean[] = {"Pierre",    "Joseph", "Baptiste", "Campbell", "Browne",
                                              "Toussaint", "Clarke", "Benoit",   "Grant",    "Augustin"};

static const char* const kFirst_indigenous_americas[] = {"Balam",     "Inti",   "Amaru",   "Nahuel", "Tupac",
                                                         "Atahualpa", "Tenoch", "Citlali", "Keme",   "Tasunka"};
static const char* const kLast_indigenous_americas[] = {"Cocom",   "Quispe",  "Mamani", "Antilef", "Yupanqui",
                                                        "Condori", "Xochitl", "Huaman", "Begay",   "Yellowbird"};

static const char* const kFirst_polynesia[] = {"Tane", "Wiremu", "Manaia", "Sione",  "Manu",
                                               "Tama", "Hemi",   "Nikau",  "Tavita", "Sio"};
static const char* const kLast_polynesia[] = {"Raukawa", "Ngata", "Te Rangi", "Tupou", "Tuala",
                                              "Rangi",   "Pene",  "Mahuta",   "Leota", "Tupua"};

static const char* const kFirst_melanesia[] = {"Jone",   "Semi",  "Pita",    "Samu",    "Meli",
                                               "Tevita", "Josua", "Viliame", "Apenisa", "Sakiusa"};
static const char* const kLast_melanesia[] = {"Nabua", "Naitasiri", "Ravoka", "Vakalalabure", "Tawake",
                                              "Koro",  "Naivalu",   "Qera",   "Radradra",     "Matawalu"};

static const char* const kFirst_western[] = {"Tyler",   "Brandon", "Ryan",   "Jason",  "Connor", "Ethan",
                                             "Dylan",   "Kyle",    "Trevor", "Logan",  "Blake",  "Derek",
                                             "Chase",   "Cody",    "Austin", "Brett",  "Travis", "Justin",
                                             "Garrett", "Shane",   "Cole",   "Tanner", "Zach",   "Jordan"};
static const char* const kLast_western[] = {"Miller",   "Carter",  "Bennett",  "Walker", "Reed",     "Parker",
                                            "Collins",  "Morgan",  "Hayes",    "Foster", "Sullivan", "Brooks",
                                            "Mitchell", "Cooper",  "Harrison", "Turner", "Anderson", "Campbell",
                                            "Morris",   "Griffin", "Palmer",   "Dawson", "Whitaker", "Stone"};

static const char* const kFirst_british[] = {"Oliver", "Harry",  "George", "Jack",   "Charlie", "Thomas", "William",
                                             "James",  "Edward", "Henry",  "Alfie",  "Freddie", "Arthur", "Hugo",
                                             "Rupert", "Callum", "Lewis",  "Gareth", "Simon",   "Nigel"};
static const char* const kLast_british[] = {"Harrington", "Whitmore",   "Ashcroft",   "Bancroft",   "Fletcher",
                                            "Crawford",   "Wellington", "Prescott",   "Hargreaves", "Sinclair",
                                            "Bennett",    "Barclay",    "Kensington", "Sutton",     "Langley",
                                            "Pritchard",  "Beaumont",   "Hawthorne",  "Redmond",    "Fairfax"};

static const char* const kFirst_israeli[] = {"Noam",  "Eitan",   "Omer",  "Yair", "Nadav", "Itai",   "Lior",
                                             "Ariel", "Yonatan", "Gilad", "Aviv", "Shai",  "Daniel", "Ido",
                                             "Tal",   "Ronen",   "Amir",  "Erez", "Yoav",  "Nir"};
static const char* const kLast_israeli[] = {
    "Cohen",   "Levi",  "Mizrahi", "Peretz", "Biton", "Dahan", "Azoulay",   "Ben-David", "Shapira", "Dayan",
    "Sharabi", "Malka", "Ohana",   "Gabay",  "Barak", "Peled", "Rosenberg", "Shalev",    "Harari",  "Ben-Ami"};

static const char* const kFriendlyPrefix[] = {"Daring", "Lucky", "Wild",      "Big",   "Fast",   "Young",  "Fearless",
                                              "Smooth", "Iron",  "Gentleman", "Slick", "Golden", "Silent", "Honest"};
static const char* const kFriendlyDescriber[] = {"The Charmer", "The Hammer", "The Fox",      "The Bull",  "The Ace",
                                                 "The Hawk",    "The Kid",    "The Lion",     "The Razor", "The Bear",
                                                 "The Wolf",    "The Prince", "The Gentleman"};
static const char* const kFriendlyTitle[] = {
    "The Brave", "The Fearless", "The Bold",    "The Great",  "The Relentless", "The Unbroken", "The Clever",
    "The Wise",  "The Mighty",   "The Patient", "The Steady", "The Quick",      "The Unshaken"};

static const char* const kHostilePrefix[] = {"Cold",     "Cruel", "Bloody",    "Greedy", "Heartless",
                                             "Ruthless", "Smug",  "Merciless", "Brutal", "Rotten"};
static const char* const kHostileDescriber[] = {
    "The Evictor",   "The Displacer",   "The Collector",  "The Jailor",        "The Seizer",
    "The Burner",    "The Whip",        "The Vanisher",   "The Fence-Builder", "The Home-Taker",
    "The Blockader", "The Roundup Man", "The Land-Taker", "The Taxman",        "The Warden",
    "The Profiteer", "The Enforcer",    "The Raider",     "The Confiscator",   "The Overseer"};
static const char* const kHostileTitle[] = {"The Oppressor", "The Occupier",    "The Land Thief", "The Butcher",
                                            "The Enforcer",  "The Tyrant",      "The Profiteer",  "The Slaver",
                                            "The Colonizer", "The Collaborator"};

// Counts come from the arrays themselves: a hand-written count
// could disagree with its table, sizeof cannot.
#define UD_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define UD_POOL_ROW(r) {#r, kFirst_##r, UD_COUNT(kFirst_##r), kLast_##r, UD_COUNT(kLast_##r)}
const NamePool kNamePools[] = {
    UD_POOL_ROW(west_africa),
    UD_POOL_ROW(east_africa),
    UD_POOL_ROW(southern_africa),
    UD_POOL_ROW(north_africa),
    UD_POOL_ROW(levant),
    UD_POOL_ROW(palestine),
    UD_POOL_ROW(lebanon),
    UD_POOL_ROW(ireland),
    UD_POOL_ROW(iraq),
    UD_POOL_ROW(arabian_peninsula),
    UD_POOL_ROW(iran),
    UD_POOL_ROW(kurdish),
    UD_POOL_ROW(turkey),
    UD_POOL_ROW(south_asia),
    UD_POOL_ROW(himalayan),
    UD_POOL_ROW(china),
    UD_POOL_ROW(korea),
    UD_POOL_ROW(japan),
    UD_POOL_ROW(mainland_southeast_asia),
    UD_POOL_ROW(maritime_southeast_asia),
    UD_POOL_ROW(central_asia),
    UD_POOL_ROW(mongolia),
    UD_POOL_ROW(caucasus),
    UD_POOL_ROW(balkans),
    UD_POOL_ROW(eastern_europe),
    UD_POOL_ROW(latin_america),
    UD_POOL_ROW(caribbean),
    UD_POOL_ROW(indigenous_americas),
    UD_POOL_ROW(polynesia),
    UD_POOL_ROW(melanesia),
    UD_POOL_ROW(western),
    UD_POOL_ROW(british),
    UD_POOL_ROW(israeli),
};
const NicknameBank kBanks[2] = {
    {kFriendlyPrefix, UD_COUNT(kFriendlyPrefix), kFriendlyDescriber, UD_COUNT(kFriendlyDescriber), kFriendlyTitle,
     UD_COUNT(kFriendlyTitle)},
    {kHostilePrefix, UD_COUNT(kHostilePrefix), kHostileDescriber, UD_COUNT(kHostileDescriber), kHostileTitle,
     UD_COUNT(kHostileTitle)},
};
#undef UD_POOL_ROW
#undef UD_COUNT
// END GENERATED NAME TABLES

const NamePool kEmptyPool = {"", nullptr, 0, nullptr, 0};

bool StartsWithThe(const char* s)
{
    return s && strncmp(s, "The ", 4) == 0;
}

const int kNameBufSize = 256;

// Append one slot to buf, collapsing every whitespace run to a single space
// and trimming the ends, so the assembled name can never carry a double space
// or a stray tab even when a caller hands in a sloppy base name.
void AppendPart(char* buf, int& len, const char* text, bool quote)
{
    if (!text)
    {
        return;
    }
    char word[kNameBufSize];
    int w = 0;
    bool pendingSpace = false;
    for (const char* p = text; *p != 0 && w < kNameBufSize - 1; p++)
    {
        const char c = *p;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            pendingSpace = w > 0;
            continue;
        }
        if (pendingSpace)
        {
            word[w++] = ' ';
            pendingSpace = false;
            if (w >= kNameBufSize - 1)
            {
                break;
            }
        }
        word[w++] = c;
    }
    word[w] = 0;
    if (w == 0)
    {
        return;
    }

    const int quoteChars = quote ? 2 : 0;
    const int sep = len > 0 ? 1 : 0;
    if (len + sep + quoteChars + w >= kNameBufSize)
    {
        return; // no truncated slot: drop it whole rather than emit half a word
    }
    if (sep > 0)
    {
        buf[len++] = ' ';
    }
    if (quote)
    {
        buf[len++] = '"';
    }
    memcpy(buf + len, word, (size_t)w);
    len += w;
    if (quote)
    {
        buf[len++] = '"';
    }
    buf[len] = 0;
}

// One LOG_WARN per (faction, token) pair, so a mission with a bad namePool key
// on every descriptor logs once per descriptor and not once per companion.
void WarnPoolOnce(const char* factionForLog, const char* token)
{
    static AutoArray<RString> seen;
    const char* faction = factionForLog && *factionForLog ? factionForLog : "<unnamed>";
    const char* value = token && *token ? token : "<none>";
    const RString mark = RString(faction) + RString("|") + RString(value);
    for (int i = 0; i < seen.Size(); i++)
    {
        if (stricmp(seen[i], mark) == 0)
        {
            return;
        }
    }
    seen.Add(mark);
    LOG_WARN(Core, "Guerrilla: faction '{}' has no usable namePool ('{}') - falling back by side", faction, value);
}

} // namespace

int NamePoolCount()
{
    return (int)(sizeof(kNamePools) / sizeof(kNamePools[0]));
}

const NamePool& NamePoolAt(int i)
{
    if (i < 0 || i >= NamePoolCount())
    {
        return kEmptyPool;
    }
    return kNamePools[i];
}

int FindNamePool(const char* region)
{
    if (!region || *region == 0)
    {
        return -1;
    }
    for (int i = 0; i < NamePoolCount(); i++)
    {
        if (stricmp(kNamePools[i].region, region) == 0)
        {
            return i;
        }
    }
    return -1;
}

const NicknameBank& Bank(NicknameTone tone)
{
    return kBanks[tone == ToneHostile ? 1 : 0];
}

RString PickFirst(int pool, unsigned long long key, unsigned channel)
{
    const NamePool& p = NamePoolAt(pool);
    if (p.nFirst <= 0)
    {
        return RString();
    }
    return RString(p.first[Roll(key, channel, p.nFirst)]);
}

RString PickLast(int pool, unsigned long long key, unsigned channel)
{
    const NamePool& p = NamePoolAt(pool);
    if (p.nLast <= 0)
    {
        return RString();
    }
    return RString(p.last[Roll(key, channel, p.nLast)]);
}

RString PickSlotWord(NicknameTone tone, int slot, unsigned long long key, unsigned channel)
{
    const NicknameBank& b = Bank(tone);
    const char* const* words = nullptr;
    int count = 0;
    switch (slot)
    {
        case SlotPrefix:
            words = b.prefix;
            count = b.nPrefix;
            break;
        case SlotDescriber:
            words = b.describer;
            count = b.nDescriber;
            break;
        case SlotTitle:
            words = b.title;
            count = b.nTitle;
            break;
        default: // the personal-name slots are drawn from the regional pool
            return RString();
    }
    if (!words || count <= 0)
    {
        return RString();
    }
    return RString(words[Roll(key, channel, count)]);
}

RString AssembleDisplayName(const NameParts& parts)
{
    const RString* slots[NNameSlots] = {&parts.prefix, &parts.first, &parts.describer, &parts.last, &parts.title};
    // Leading-sigil guard: drop the leading slot and retry rather than ship a
    // name a marker label would blank.
    for (int start = 0; start < NNameSlots; start++)
    {
        char buf[kNameBufSize];
        buf[0] = 0;
        int len = 0;
        for (int i = start; i < NNameSlots; i++)
        {
            const bool quote = i == SlotDescriber && StartsWithThe(slots[i]->Data());
            AppendPart(buf, len, slots[i]->Data(), quote);
        }
        if (len == 0)
        {
            return RString();
        }
        if (buf[0] != '@' && buf[0] != '$')
        {
            return RString(buf);
        }
    }
    return RString();
}

int ResolveNamePool(const char* namePoolValue, const char* side, const char* factionForLog)
{
    if (namePoolValue && *namePoolValue != 0)
    {
        const int named = FindNamePool(namePoolValue);
        if (named >= 0)
        {
            return named;
        }
    }
    WarnPoolOnce(factionForLog, namePoolValue);

    int index = -1;
    if (side && *side != 0)
    {
        if (stricmp(side, "WEST") == 0)
        {
            index = FindNamePool("western");
        }
        else if (stricmp(side, "EAST") == 0)
        {
            index = FindNamePool("eastern_europe");
        }
    }
    if (index < 0)
    {
        index = FindNamePool("levant"); // GUER and every unknown side
    }
    if (index < 0)
    {
        index = 0; // never -1: a caller indexes the table with this
    }
    return index;
}

} // namespace Poseidon::Guerrilla

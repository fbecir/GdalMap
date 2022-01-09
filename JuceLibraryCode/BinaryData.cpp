/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== Translation_fr.txt ==================
static const unsigned char temp_binary_data_0[] =
"language: French\r\n"
"countries: fr be mc ch lu\r\n"
" \r\n"
"\"Name\" = \"Nom\"\r\n"
"\"Value\" = \"Valeur\"\r\n"
"\"Add vector data\" = \"Ajouter des donn\xc3\xa9""es vectorielles\"\r\n"
"\"Add raster data\" = \"Ajouter des images\"\r\n"
"\"Quit\" = \"Quitter\"\r\n"
"\"About GDAL\" = \"A propos de GDAL\"\r\n"
"\"Selection\" = \"S\xc3\xa9lection\"\r\n"
"\"File\" = \"Fichier\"\r\n"
"\"Edit\" = \"Edition\"\r\n"
"\"Pen\" = \"Stylo\"\r\n"
"\"Brush\" = \"Pinceau\"\r\n"
"\"Width\" = \"Epaisseur\"";

const char* Translation_fr_txt = (const char*) temp_binary_data_0;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x6c6745eb:  numBytes = 365; return Translation_fr_txt;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "Translation_fr_txt"
};

const char* originalFilenames[] =
{
    "Translation_fr.txt"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
    {
        if (namedResourceList[i] == resourceNameUTF8)
            return originalFilenames[i];
    }

    return nullptr;
}

}

#ifndef _IMG_H_
#define _IMG_H_

// ----------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C"
{
#endif

// Magic Words
#define MAGIC_WORD_IMG (0xCAFEFECAUL)
#define MAGIC_WORD_BL (0xCACEB0CAUL)
#define MAGIC_WORD_APP (0xFABADEDEUL)

#define MAGIC_WORD_BL_END (0xDAC00DACUL)
#define MAGIC_WORD_APP_END (0xDEADC0DEUL)



//! Current binary image version
#ifndef IMG_MAJOR
	#define IMG_MAJOR 0
#endif
#ifndef IMG_MINOR
	#define IMG_MINOR 0
#endif
#ifndef IMG_REV
	#define IMG_REV 0
#endif

struct sw_version_s
{
	unsigned char major; //!< version major number (BCD).
	unsigned char minor; //!< version minor number (BCD).
	unsigned char rev;   //!< version revision number (BCD).
};

typedef enum
{
	IMG_TYPE_BL,
	IMG_TYPE_APP,
	IMG_TYPE_CONF,
} img_type_e;

typedef struct
{
	unsigned int  ver;      //!< version
	unsigned int  bnum;     //!< build number.
	unsigned char hash[32]; //!< sha256
} bl_header_t;

typedef struct
{
	unsigned int  ver;      //!< version
	unsigned int  bnum;     //!< build number.
	unsigned char hash[32]; //!< sha256
	unsigned int  beg;      //!< The image start point (first address)
	unsigned int  sz;       //!< The image size (in bytes)
	unsigned int  got;      //!< The image GOT address
	unsigned int  entry;    //!< The image entry point
} app_header_t;

typedef struct
{

} conf_header_t;

typedef struct {
	unsigned int magic;      //!< Magic number
	unsigned char hdr_sz;    //!< The header size, including the hdr_sz and magic field
	unsigned char type;      //!< img type
	unsigned char extend[58]; //!< Extra flag
} img_header_t;

#if defined(__cplusplus)
}
#endif

// ----------------------------------------------------------------------------

#endif // _IMG_H_

//*** debug ****************
// 0=no Debug; 
// 1=values only; 
// 2=values and info and P-buffer
// 3=values and info and T+R+P-buffer
#define DEBUG_SMA 1

#define LOOPTIME_SEC 30

// SMA login password for UG_USER or UG_INSTALLER always 12 char. Unused=0x00
#define USERGROUP UG_USER
const char SmaInvPass[]={'P','a','s','s','w','o','r','d',0,0,0,0}; 

// SMA blutooth address -> adapt to your system
uint8_t SmaBTAddress[6]= {0x00, 0x80, 0x25, 0x29, 0xEB, 0xD3}; // my SMA SMC6000TL 00:80:25:29:eb:d3 
 
// Webserver -> adapt to your system
#define SMA_WEBSERVER
#ifdef SMA_WEBSERVER
  const char *ssid        = "MySsid";
  const char *password    = "MyPassord";
#endif


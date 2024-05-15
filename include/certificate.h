#ifndef _CERTIFICATE_H_
#define _CERTIFICATE_H_

/*
#raspi4# echo -n | openssl s_client -servername connect.prusa3d.com -connect connect.prusa3d.com:443 | sed --quiet --expression='/-.BEGIN/,/-.END/p'
#raspi4# echo -n | openssl s_client -showcerts -connect connect.prusa3d.com:443 | sed --quiet --expression='/-.BEGIN/,/-.END/p'
  */

const char root_CAs[] PROGMEM = R"rawliteral(
-----BEGIN CERTIFICATE-----
MIIE8jCCA9qgAwIBAgISA5WPixrzuzQqW6S4eTCMs/QwMA0GCSqGSIb3DQEBCwUA
MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD
EwJSMzAeFw0yNDAzMzExMjA3MzJaFw0yNDA2MjkxMjA3MzFaMB4xHDAaBgNVBAMT
E2Nvbm5lY3QucHJ1c2EzZC5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK
AoIBAQC913NEofbO3KUwed8auduRGIsnJDOPcv0lOdgGmm8awPHPt7jXREnMEyQV
gLVTQ43GddrCWN2GNzY4LMX5s6jZxsZrDQhPyJDqTA2mf2y5JEaNt6pphmW+d37R
sy7vejgxMmN9OIf7jX5zjJ4aDoRfzaSB36GHyP5rqXEtYWbIVMW8niBo+qdywyJb
7FvgWb8ddJMdl/vBih+arR979EeAUXLk73uNGK9T1qZYlOmZ6+Yg6tLlNvoSudqH
l2y7BpicW9W9B8GhQDGhaFcmc3kJlTg/RZ617Th3+L4m3NYXMCppIdSD9Y7HSGJG
0yg4DYdMl0lbMR+XeMyDIbFmV+XhAgMBAAGjggIUMIICEDAOBgNVHQ8BAf8EBAMC
BaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMAwGA1UdEwEB/wQCMAAw
HQYDVR0OBBYEFC4UuVBhFH+WQtKmHKi7f3ZEW4bWMB8GA1UdIwQYMBaAFBQusxe3
WFbLrlAJQOYfr52LFMLGMFUGCCsGAQUFBwEBBEkwRzAhBggrBgEFBQcwAYYVaHR0
cDovL3IzLm8ubGVuY3Iub3JnMCIGCCsGAQUFBzAChhZodHRwOi8vcjMuaS5sZW5j
ci5vcmcvMB4GA1UdEQQXMBWCE2Nvbm5lY3QucHJ1c2EzZC5jb20wEwYDVR0gBAww
CjAIBgZngQwBAgEwggEDBgorBgEEAdZ5AgQCBIH0BIHxAO8AdgA7U3d1Pi25gE6L
MFsG/kA7Z9hPw/THvQANLXJv4frUFwAAAY6Un1BTAAAEAwBHMEUCIQCoXoxmXQ0F
bk4Uwsig9jMTT9xz2qneCQa75orGJPjGxgIgSH81c5btJgvBGOXksf8O9hpU7g8G
2NjJr1f0UxczIXcAdQDuzdBk1dsazsVct520zROiModGfLzs3sNRSFlGcR+1mwAA
AY6Un1BfAAAEAwBGMEQCIEfpKGsm7ucXRA8OKtf2FA7jjkh3scrtLlPVKmRm5AVE
AiBQ9KgGAzdgrPHSt7QVkeVgzEH4u8HadLBSIgtdTU0Z4DANBgkqhkiG9w0BAQsF
AAOCAQEAkdLOXMI03XaGbSk65w/U8IuviYlSmDFd2dSG1KVZHMbLQe+pIWVGlZK6
DS/Nb+LodWiIIdbHBCdjRbk5nb0oIXTnFbaLfC+IEADRwxyMMNOnIL0Fs5cr+WDe
rx1KeDCYQrV5Qzai6ANzAetOy/TqIQmVtejM837LIZzbN963jU+TqlzQytfxxeQe
W/ZKQ9qlLI6804QMPGvsFZhiRVaJjndspBzBIN2RiWwqgWqCr+57oEbQJsATF7hH
ze2yMgg5WmMYOXRmd7fugLNR53JdWpeWogXRmnWc0DZ/M68rgcqpACpCSFzyIkUE
lkKX45bNBdAhLDz/0miY0SZwwVH7eQ==
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw
WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg
RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK
AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP
R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx
sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm
NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg
Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG
/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB
Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA
FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw
AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw
Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB
gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W
PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl
ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz
CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm
lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4
avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2
yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O
yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids
hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+
HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv
MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX
nLRbwHOoq7hHwg==
-----END CERTIFICATE-----
)rawliteral";

#endif

@echo off
echo Generating self-signed certificate for SSL server...
echo.
echo This will create server.crt and server.key files
echo.

REM Generate private key
openssl genrsa -out server.key 2048

REM Generate certificate signing request
openssl req -new -key server.key -out server.csr -subj "/CN=localhost"

REM Generate self-signed certificate
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt

echo.
echo Certificate generated successfully!
echo Files: server.crt, server.key
echo.
pause


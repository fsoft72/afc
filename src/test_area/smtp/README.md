# SMTP Test

This test demonstrates the AFC SMTP client library with configuration loaded from a settings file.

## Setup

1. Copy the example settings file:
   ```bash
   cp settings.txt.example settings.txt
   ```

2. Edit `settings.txt` with your SMTP credentials:
   - For Amazon SES, get your SMTP credentials from AWS Console
   - For Gmail, use an App Password (not your regular password)
   - For other providers, check their SMTP documentation

3. Build the test:
   ```bash
   make
   ```

4. Run the test:
   ```bash
   ./test_01
   ```

## Configuration Options

### Required Settings

- `host` - SMTP server hostname (e.g., email-smtp.us-east-1.amazonaws.com)
- `port` - SMTP server port (25, 587, or 465)
- `username` - SMTP authentication username
- `password` - SMTP authentication password
- `from` - Sender email address
- `to` - Recipient email address

### Optional Settings

- `use_tls` - Set to `1` to use STARTTLS (recommended for port 587)
- `use_ssl` - Set to `1` to use direct SSL (for port 465)
- `auth_method` - Authentication method:
  - `auth_plain` - AUTH PLAIN (default, Amazon SES compatible)
  - `auth_login` - AUTH LOGIN (Amazon SES compatible)
  - `auth_none` - No authentication (local servers only)

## Common SMTP Configurations

### Amazon SES (Port 587 with STARTTLS)
```
host=email-smtp.us-east-1.amazonaws.com
port=587
use_tls=1
auth_method=auth_login
```

### Gmail (Port 587 with STARTTLS)
```
host=smtp.gmail.com
port=587
use_tls=1
auth_method=auth_plain
```

### Direct SSL (Port 465)
```
host=smtp.example.com
port=465
use_ssl=1
auth_method=auth_login
```

## Troubleshooting

- **Connection failed**: Check host and port, verify firewall settings
- **Authentication failed**: Verify username/password, try different auth_method
- **TLS errors**: Ensure OpenSSL is installed and up to date
- **Send failed**: Check that from/to addresses are valid and authorized

## What It Tests

The test program:
1. Reads configuration from `settings.txt`
2. Creates an SMTP client instance
3. Connects to the SMTP server
4. Authenticates using provided credentials
5. Sends a test email
6. Closes the connection cleanly

This verifies that the AFC SMTP library is working correctly with your SMTP provider.

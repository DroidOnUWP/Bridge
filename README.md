# Android bridge for UWP applications
Allows run Android Runtime as a UWP app.

[![Join the chat at https://gitter.im/DroidOnUWP/Bridge](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/DroidOnUWP/Bridge?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


#Status
- currently only ARM7 is supported
- fork does not work correctly - needs to do some more research and find way, that works under UWP
- Android Runtime initializes, but than it crashes when launching app
- Android 7.1.1 r13 is used
- Visual Studio 2015 supported

# Build
1. Clone repo including submodules
2. Install Angle templates(run Angle\templates\install.bat) and build Angle solution (Angle\winrt\10\gyp\All.vcxproj) separately
3. Open Bridge.sln
4. Choose target ARM device
5. Build

#FAQ
Q: When debugging in Visual Studio Access violation exception occurs. How to fix it?
A: Uncheck "Break when this exception type is thrown", so FLinux exception handler can work correctly
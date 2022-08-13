# Platform checks

<table>
   <tr>
     <th>Platform</th>
     <th>#if check</th>
     <th>Platform specific dir</th>
   </tr>
   <tr>
     <td>
       Windows
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_WIN
        // ...
        #endif
       @endcode
     </td>
     <td>
       `Platform/win32`
     </td>
   </tr>

   <tr>
     <td>
       Linux (not Android)
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_LINUX
        // ...
        #endif
       @endcode
     </td>
     <td>
       `Platform/linux`
     </td>
   </tr>

   <tr>
     <td>
       macOS
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_MACOS
        // ...
        #endif
       @endcode
     </td>
     <td>
       `Platform/macos`
     </td>
   </tr>

   <tr>
     <td>
       Android
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_ANDROID
        // ...
        #endif
       @endcode
     </td>
     <td>
       `Platform/android`
     </td>
   </tr>

   <tr>
     <td>
       iOS (both iPhone and iPad)
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_IOS
        // ...
        #endif
       @endcode
     </td>
     <td>
       `Platform/ios`
     </td>
   </tr>

   <tr>
     <td>
       Apple (macOS, iOS)
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_APPLE
        // ...
        #endif
       @endcode
     </td>
     <td>
       `Platform/apple`
     </td>
   </tr>

   <tr>
     <td>
       Unix (Linux, Android, macOS, iOS)
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_UNIX
        // ...
        #endif
       @endcode
     </td>
     <td>
       `Platform/unix`
     </td>
   </tr>
 </table>


# Platform specific sources

With AUI, the platform dependent code can be placed in `src/Platform/<PLATFORM_NAME>` dir, where `<PLATFORM_NAME>` is
one of the supported platforms (see the table above).


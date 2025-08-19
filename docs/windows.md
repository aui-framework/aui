# Windows (operating system)

Windows is a family of operating systems developed by Microsoft. It was first released in 1985 and has since become the
most popular desktop operating system worldwide, with a market share of about 70%. The latest version for consumer PCs
and tablets is Windows 11, while certain older versions are still supported and maintained by Microsoft.


## Key considerations

- You'll need to [make an installer and potentially a portable package](PACKAGING_FOR_WINDOWS) with
  [auto updating](updater.md) to distribute your application despite Windows offers an "official" software repository
- Windows is the only system that is not Unix-like, despite it provides POSIX-compliant APIs
- You might need to sign your executables, otherwise Windows would probably treat your application as a virus and make
  launching your executables a lot harder

## AUI implementation specifics
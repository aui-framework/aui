# Forking AUI

It's a common scenario when you want to fork AUI's repository. Reasons might include:

1. **Zero trust policy**. Of course, we value our reputation, but it's completely okay you might want to mitigate
   security risks by obtaining full control of the code that appears in your project.
2. **AUI development**. You want to improve AUI or make your own version of AUI in your project.

This page describes how to do so.

You can fork AUI to whatever source hosting provider you want, but in general, it's recommended to fork AUI on GitHub:

1. AUI's original infrastructure setup is on GitHub already. You'll benefit from complete build, docs gen and testing
   pipelines without asking your devops to support them on your behalf.
   
   Additionally, you won't lose [PREBUILT_PACKAGES] "AUI.Boot's precompiled binaries" feature by publishing your fork
   on GitHub and enabling GitHub Actions (AUI's license does not obligate you to publish byproducts, e.g., compiled
   binaries or documentation).
2. As soon as you publish your project with modified version of AUI, you'll need to publish changes to AUI anyway
   (a requirement of MPL2).

## Steps

1. Mirror/clone/fork AUI's repo from https://github.com/aui-framework/aui.
2. If you follow **zero trust policy**, you will need to fork all AUI's dependencies to your controlled zone as well.
   You can search them by `auib_import` keyword. Once discovered, fork the dependencies and update URLs in your AUI repo
   accordingly.
3. (Optionally) update URLs in `.github` directory in your AUI repo or make adjustments that suit your particular
   project needs.
4. Don't forget to update URL to AUI.Boot and `auib_import` boilerplate in your project.

Once done, you don't depend on 3rd parties anymore. It is now up to you when and how change AUI's code.
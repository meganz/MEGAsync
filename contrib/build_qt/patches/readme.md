PATCHES:
This folder contains the patches to apply before building Qt. Patches will be applied in ascending order. Not all patches need to ba applied to all platforms: use "all", "win" or "macx" to indicate if they should be applied to all platforms, only on Windows, or only on MacOs.
It is organized like this:

Tree:
<Qt Version>
    ├───<qt submodule>
    │       <order>.<os>.<name>
    │       <order>.<os>.<name>
    │       [...]
    │
    ├───<qt submodule>
    │       <order>.<os>.<name>
    │       <order>.<os>.<name>
    │       [...]
    [...]

With:
<order> : numerical value, 2 digits, 0 padded. Patches will be applied in ascending order.
<os> : "all", "win" or "macx" to apply to all platform build, only Windows or only MacOs.
<name> : patch name (+ extension).

Example:

5.15.11
    ├───qtbase
    │       00.all.CVE-2023-24607-qtbase-5.15.diff
    │       01.all.CVE-2023-32762-qtbase-5.15.diff
    │       02.all.CVE-2023-32763-qtbase-5.15.diff
    │       03.all.CVE-2023-33285-qtbase-5.15.diff
    │       04.all.CVE-2023-34410-qtbase-5.15.diff
    │       05.all.CVE-2023-37369-qtbase-5.15.diff
    │       06.all.CVE-2023-38197-qtbase-5.15.diff
    │       07.all.CVE-2023-43114-5.15.patch
    │       08.macx.QTBUG-117225-cdf64b0.diff
    │       09.macx.QTBUG-117484-Out-standard-lib-memory-resource.patch
    │
    ├───qtimageformats
    │       00.win.CVE-2023-4863-5.15.patch
    │
    ├───qtmultimedia
    │       00.macx.Xcode15-unary_function-avfcamerautility.patch
    │
    └───qtsvg
            00.all.CVE-2023-32573-qtsvg-5.15.diff
			
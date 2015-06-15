<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="zh_TW" sourcelanguage="en">
<context>
    <name>AccountDetailsDialog</name>
    <message>
        <source>Account usage details</source>
        <translatorcomment>Title of the dialog that displays account usage details for the actual logged user.</translatorcomment>
        <translation>賬戶使用量詳細資料</translation>
    </message>
    <message>
        <source>Cloud Drive</source>
        <translatorcomment>Label for Cloud Drive space used. Maintain capital letters in each word.(Max 18 characters)</translatorcomment>
        <translation type="obsolete">雲端硬碟</translation>
    </message>
    <message>
        <source>Inbox</source>
        <translatorcomment>Label for Inbox space used. Maintain capital letters in each word.(Max 18 characters)</translatorcomment>
        <translation type="obsolete">收件夾</translation>
    </message>
    <message>
        <source>Rubbish Bin</source>
        <translatorcomment>Label for Rubbish Bin space used. Maintain capital letters in each word.(Max 18 characters)</translatorcomment>
        <translation type="obsolete">垃圾夾</translation>
    </message>
    <message>
        <source>Storage</source>
        <translatorcomment>Label for Storage space used. Maintain capital letters in each word.</translatorcomment>
        <translation type="obsolete">儲存空間</translation>
    </message>
    <message>
        <source>Files</source>
        <translatorcomment>Label for Files. Maintain capital letters in each word.</translatorcomment>
        <translation type="obsolete">文件</translation>
    </message>
    <message>
        <source>Folders</source>
        <translatorcomment>Label for Folders. Maintain capital letters in each word.</translatorcomment>
        <translation type="obsolete">資料夾</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translatorcomment>Label for Refresh button. Used to retrieve all usage account details from the server.</translatorcomment>
        <translation>刷新</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation>OK</translation>
    </message>
    <message>
        <source>Loading...</source>
        <translatorcomment>Label to show when an account detail request is waiting for the server response.</translatorcomment>
        <translation type="unfinished">Loading...</translation>
    </message>
</context>
<context>
    <name>BindFolderDialog</name>
    <message>
        <source>Add synchronized folder</source>
        <translatorcomment>Title of the dialog displayed when an user is creating a new synchronized folder.</translatorcomment>
        <translation>新增同步資料夾</translation>
    </message>
    <message>
        <source/>
        <translation></translation>
    </message>
    <message>
        <source>Please select a local folder and a MEGA folder</source>
        <translatorcomment>Message displayed when an user is adding a synchronized folder an either local or remote folder are empty.</translatorcomment>
        <translation>請選擇一個本機資料夾與一個MEGA資料夾以便同步</translation>
    </message>
    <message>
        <source>The selected local folder is already synced</source>
        <translatorcomment>Message displayed when an user is adding a local folder wich is already synced.</translatorcomment>
        <translation>選取的本機資料夾已經同步</translation>
    </message>
    <message>
        <source>A synced folder cannot be inside another synced folder</source>
        <translatorcomment>Message displayed when an user is adding nested local folders for synchronization.</translatorcomment>
        <translation>同步資料夾不能屬於另一同步資料夾內</translation>
    </message>
    <message>
        <source>The selected MEGA folder is already synced</source>
        <translatorcomment>Message displayed when an user is adding an already synchronized remote folder.</translatorcomment>
        <translation>選取的MEGA資料夾已經同步</translation>
    </message>
    <message>
        <source>Full account syncing is only possible without any selective syncs</source>
        <translation type="obsolete">全帳號同步僅能於未有任何同步資料夾的狀態下使用</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="obsolete">警告</translation>
    </message>
    <message>
        <source>Sync name</source>
        <translatorcomment>Title of the dialog displayed when a user is using a name for a local folder that is already used.</translatorcomment>
        <translation>同步名稱</translation>
    </message>
    <message>
        <source>The name &quot;%1&quot; is already in use for another sync
Please enter a different name to identify this synced folder:</source>
        <translatorcomment>Detailed message displayed when a user is using a name for a local folder that is already used. Preserve &quot;%1&quot; code because is used to indicate the local folder at runtime.</translatorcomment>
        <translation>The name &quot;%1&quot; is already in use for another syncPlease enter a different name to identify this synced folder:</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Error</source>
        <translatorcomment>Label for status of an error performing an operation.</translatorcomment>
        <translation>錯誤</translation>
    </message>
    <message>
        <source>Local folder too large (this version is limited to %1 folders or %2 files.
Please, select another folder.</source>
        <translation type="obsolete">Local folder too large (this version is limited to %1 folders or %2 files.Please, select another folder.</translation>
    </message>
    <message>
        <source>You are trying to sync an extremely large folder.
To prevent the syncing of entire boot volumes, which is inefficient and dangerous,
we ask you to start with a smaller folder and add more data while MEGAsync is running.</source>
        <translation type="obsolete">You are trying to sync an extremely large folder.To prevent the syncing of entire boot volumes, which is inefficient and dangerous,we ask you to start with a smaller folder and add more data while MEGAsync is running.</translation>
    </message>
</context>
<context>
    <name>CrashReportDialog</name>
    <message>
        <source>Error report</source>
        <translatorcomment>Title of the dialog displayed when an crash report occurred.</translatorcomment>
        <translation>錯誤報告</translation>
    </message>
    <message>
        <source>MEGAsync has detected a problem. These are the details:</source>
        <translatorcomment>Label to indicate that a crash has occured and the detailed stacktrace of the problem.</translatorcomment>
        <translation>MEGAsync偵測到問題。詳細內容如下:</translation>
    </message>
    <message>
        <source>You can help us to improve MEGAsync by sending this error report. It doesn&apos;t contain any personal information. If you want to give us more details, please write them below:</source>
        <translatorcomment>Label to indicate if the user wants to add some more details to attach to the crash report.</translatorcomment>
        <translation>您可藉由傳送錯誤報告來協助我們改善MEGAsync。這將不會包含任何個人資料。如果您願意提供更多細節給我們，請列於下方:</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Send report</source>
        <translatorcomment>Label for Send report button.</translatorcomment>
        <translation>傳送報告</translation>
    </message>
</context>
<context>
    <name>DownloadFromMegaDialog</name>
    <message>
        <source>Download from MEGA</source>
        <translatorcomment>Label and Title of the dialog displayed when a user is trying to retrieve a file/folder from MEGA.(MAX 20 characters)</translatorcomment>
        <translation>從MEGA下載</translation>
    </message>
    <message>
        <source>Please select the download folder for your files:</source>
        <translatorcomment>Label to inform the user of the destination local folder for the files to be downloaded (MAX 50 characters)</translatorcomment>
        <translation>請選擇文件下載資料夾：</translation>
    </message>
    <message>
        <source>Local folder:</source>
        <translatorcomment>Label to indicate the user the local folder in which the selected files/folders will be downloaded (String short as possible)</translatorcomment>
        <translation>本機資料夾:</translation>
    </message>
    <message>
        <source>Always download to this destination</source>
        <translatorcomment>Label to let the user select a default download folder with a checkbox.</translatorcomment>
        <translation>總是下載到這個目的地</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Choose</source>
        <translatorcomment>Button label to select the download folder (String as short as possible)</translatorcomment>
        <translation>選擇</translation>
    </message>
    <message>
        <source>Select local folder</source>
        <translatorcomment>Title of the dialog in which the user select the download folder.</translatorcomment>
        <translation>選擇本機資料夾</translation>
    </message>
    <message>
        <source>Error</source>
        <translatorcomment>Title of dialog if an error occurs selecting the download local folder.</translatorcomment>
        <translation>錯誤</translation>
    </message>
    <message>
        <source>You don&apos;t have write permissions in this local folder.</source>
        <translatorcomment>Label to indicate that a user don&apos;t have write permissions in the selected local folder to download files/folders from MEGA.</translatorcomment>
        <translation>您沒有這個本地資料夾的編寫權限。</translation>
    </message>
</context>
<context>
    <name>FolderBinder</name>
    <message>
        <source>Local folder:</source>
        <translatorcomment>Label to indicate the user the local folder for a synchronization (String short as possible)</translatorcomment>
        <translation>本機資料夾:</translation>
    </message>
    <message>
        <source>MEGA folder:</source>
        <translatorcomment>Label to indicate the user the MEGA folder for a synchronization (String short as possible)</translatorcomment>
        <translation>MEGA資料夾:</translation>
    </message>
    <message>
        <source>Select local folder</source>
        <translatorcomment>Title of the dialog in which the user select the local default folder.for a synchronization.</translatorcomment>
        <translation>選擇本機資料夾</translation>
    </message>
    <message>
        <source>Choose</source>
        <translatorcomment>Button label to select the folders (local and remote) for a synchronization (String as short as possible)</translatorcomment>
        <translation>選擇</translation>
    </message>
    <message>
        <source>Warning</source>
        <translatorcomment>Label to indicate a waring during the process of stablish a synchronization.</translatorcomment>
        <translation>警告</translation>
    </message>
    <message>
        <source>You don&apos;t have write permissions in this folder.</source>
        <translation type="obsolete">您沒有這個資料夾的編寫權限。</translation>
    </message>
    <message>
        <source>MEGAsync won&apos;t be able to download anything here.</source>
        <translatorcomment>Label to indicate that MEGAsync won&apos;t be able to download anything due to the user doesn&apos;t have writhe permissions on the local selected folder.</translatorcomment>
        <translation>MEGAsync 無法從這裡下載任何資料</translation>
    </message>
    <message>
        <source>Do you want to continue?</source>
        <translatorcomment>Label to indicate if the user wants to continue in spite of the problem occured.</translatorcomment>
        <translation>您要繼續嗎？</translation>
    </message>
    <message>
        <source>You don&apos;t have write permissions in this local folder.</source>
        <translatorcomment>Label to indicate that a user don&apos;t have write permissions in the selected local folder for a synchronization.</translatorcomment>
        <translation>您沒有這個本地資料夾的編寫權限。</translation>
    </message>
    <message>
        <source>You are trying to sync an extremely large folder.
To prevent the syncing of entire boot volumes, which is inefficient and dangerous,
we ask you to start with a smaller folder and add more data while MEGAsync is running.</source>
        <translatorcomment>Label to inform a user about the fact of syncing a extremely large folder and the possible drawbacks that could arise.</translatorcomment>
        <translation>You are trying to sync an extremely large folder.To prevent the syncing of entire boot volumes, which is inefficient and dangerous,we ask you to start with a smaller folder and add more data while MEGAsync is running.</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">錯誤</translation>
    </message>
    <message>
        <source>You can not sync a shared folder without Full Access permissions</source>
        <translation type="unfinished">You can not sync a shared folder without Full Access permissions</translation>
    </message>
</context>
<context>
    <name>ImportMegaLinksDialog</name>
    <message>
        <source>Import links</source>
        <translatorcomment>Label and Title of the dialog displayed when a user is trying to import MEGA links .(MAX 20 characters)</translatorcomment>
        <translation>匯入連結</translation>
    </message>
    <message>
        <source>Download to my computer</source>
        <translatorcomment>Label to indicate if the user wants to download the imported MEGA link(s) to his computer.</translatorcomment>
        <translation>下載至我的電腦</translation>
    </message>
    <message>
        <source>Import to my cloud drive</source>
        <translatorcomment>Label to indicate if the user wants to import the selected MEGA link(s) to his cloud drive.</translatorcomment>
        <translation>匯入至我的雲端硬碟</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>/MEGAsync Downloads</source>
        <translation type="obsolete">/MEGAsync Downloads</translation>
    </message>
    <message>
        <source>/MEGAsync Imports</source>
        <translatorcomment>Default created folder for imported links at the user Cloud Drive.</translatorcomment>
        <translation>/MEGAsync Imports</translation>
    </message>
    <message>
        <source>Select local folder</source>
        <translatorcomment>Title of the dialog in which the user select the local default folder.for downloads.</translatorcomment>
        <translation>選擇本機資料夾</translation>
    </message>
    <message>
        <source>Decryption error</source>
        <translatorcomment>Label to indicate a Decryption error due to a problem with the KEYS</translatorcomment>
        <translation>解密錯誤</translation>
    </message>
    <message>
        <source>Not found</source>
        <translatorcomment>Label to indicate that an imported link can&apos;t be found.</translatorcomment>
        <translation>未找到</translation>
    </message>
    <message>
        <source>Warning</source>
        <translatorcomment>Label to indicate a warning during the process of importing a MEGA link.</translatorcomment>
        <translation>警告</translation>
    </message>
    <message>
        <source>You are about to import this file to a synced folder.
If you enable downloading, the file will be duplicated on your computer.
Are you sure?</source>
        <translation>You are about to import this file to a synced folder.If you enable downloading, the file will be duplicated on your computer.Are you sure?</translation>
    </message>
    <message>
        <source>You are about to import these files to a synced folder.
If you enable downloading, the files will be duplicated on your computer.
Are you sure?</source>
        <translation>You are about to import these files to a synced folder.If you enable downloading, the files will be duplicated on your computer.Are you sure?</translation>
    </message>
    <message>
        <source>Choose</source>
        <translation>選擇</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <source>You don&apos;t have write permissions in this local folder.</source>
        <translation>您沒有這個本地資料夾的編寫權限。</translation>
    </message>
</context>
<context>
    <name>InfoDialog</name>
    <message>
        <source>MEGAsync is up to date</source>
        <translatorcomment>Label to indicate that MEGAsync is at state of up-to-date (String as short as possible)</translatorcomment>
        <translation>MEGAsync是最新的</translation>
    </message>
    <message>
        <source>RECENTLY UPDATED</source>
        <translatorcomment>Label to indicate the files recently updated. Keep capital letters.</translatorcomment>
        <translation>最近更新</translation>
    </message>
    <message>
        <source>Usage: Data temporarily unavailable</source>
        <translatorcomment>Label to indicate that the usage data of the account is temporarily unavailable (String as short as possible)</translatorcomment>
        <translation>用量:數據暫時無法取得</translation>
    </message>
    <message>
        <source>Syncs</source>
        <translatorcomment>Button label to show all synchronizations that the user has stablish (String as short as possible).</translatorcomment>
        <translation>同步</translation>
    </message>
    <message>
        <source>MEGA website</source>
        <translatorcomment>Label with an URL link to MEGA website</translatorcomment>
        <translation>MEGA網站</translation>
    </message>
    <message>
        <source>%1 of %2</source>
        <translatorcomment>Label to keep the count of pending and total files. Preserve %1 and %2 codes beacuse they are used to include the number of pending and total files.</translatorcomment>
        <translation>%1 / %2</translation>
    </message>
    <message>
        <source>Usage: %1</source>
        <translatorcomment>Label to indicate the actual usage of the current account. Preserve %1 code beacuse is used to include the amount of space utilized.</translatorcomment>
        <translation>用量: %1</translation>
    </message>
    <message>
        <source>%1 of %2 (%3/s)</source>
        <translatorcomment>Label to keep the count of pending and total files. Preserve %1, %2 and %3 codes beacuse they are used to include the number of pending, total files and actual speed.</translatorcomment>
        <translation>%1 /%2 (%3/s)</translation>
    </message>
    <message>
        <source>%1 of %2 (paused)</source>
        <translatorcomment>Label to keep the count of pending and total files. Preserve %1 and %2 codes beacuse they are used to include the number of pending and total files.</translatorcomment>
        <translation>%1 /%2 (暫停)</translation>
    </message>
    <message>
        <source>File transfers paused</source>
        <translatorcomment>Label to indicate that the state of transfers is actually paused.</translatorcomment>
        <translation>檔案傳輸暫停</translation>
    </message>
    <message>
        <source>MEGAsync is scanning</source>
        <translatorcomment>Label to indicate that MEGAsync is at state of scanning inside synced folders (String as short as possible)</translatorcomment>
        <translation>MEGAsync掃描中</translation>
    </message>
    <message>
        <source>All transfers have been completed</source>
        <translatorcomment>Label to indicate that all pending transfers are already completed</translatorcomment>
        <translation>所有傳輸已完成</translation>
    </message>
    <message>
        <source>Cancel all downloads</source>
        <translatorcomment>Label to cancel all pending downloads.(String as short as possible)</translatorcomment>
        <translation>取消所有下載</translation>
    </message>
    <message>
        <source>Cancel download</source>
        <translatorcomment>Label to cancel the actual download.(String as short as possible)</translatorcomment>
        <translation>取消下載</translation>
    </message>
    <message>
        <source>Cancel all uploads</source>
        <translatorcomment>Label to cancel all pending uploads.(String as short as possible)</translatorcomment>
        <translation>取消所有上傳</translation>
    </message>
    <message>
        <source>Cancel upload</source>
        <translatorcomment>Label to cancel the actual upload. (String as short as possible)</translatorcomment>
        <translation>取消上傳</translation>
    </message>
    <message>
        <source>Add Sync</source>
        <translatorcomment>Button label to add a new synchronization (String as short as possible)</translatorcomment>
        <translation>新增同步</translation>
    </message>
    <message>
        <source>one file at %1/s</source>
        <translation type="obsolete">一個檔案於 %1/s</translation>
    </message>
    <message>
        <source>one file (paused)</source>
        <translation type="obsolete">一個檔案(暫停)</translation>
    </message>
    <message>
        <source>%1 of %2 files at %3/s</source>
        <translation type="obsolete">%1 / %2 檔案於 %3/s</translation>
    </message>
    <message>
        <source>%1 of %2 files (paused)</source>
        <translation type="obsolete">%1 / %2檔案(暫停)</translation>
    </message>
    <message>
        <source>Total Remaining: </source>
        <translation type="obsolete">尚餘:</translation>
    </message>
    <message>
        <source>Downloading </source>
        <translatorcomment>Label to indicate that MEGAsync is Downloading files (String as short as possible and keep capitar letters)</translatorcomment>
        <translation>下載中</translation>
    </message>
    <message>
        <source>Uploading </source>
        <translatorcomment>Label to indicate that MEGAsync is Uploading files (String as short as possible and keep capitar letters)</translatorcomment>
        <translation>上傳中</translation>
    </message>
    <message>
        <source>MEGAsync is waiting</source>
        <translatorcomment>Label to indicate that MEGAsync is at a wait state (String as short as possible)</translatorcomment>
        <translation>MEGAsync 正在等待</translation>
    </message>
    <message>
        <source>MEGAsync is starting</source>
        <translatorcomment>Label to indicate that MEGAsync is at start state (String as short as possible)</translatorcomment>
        <translation>MEGAsync 開始同步中。</translation>
    </message>
</context>
<context>
    <name>InfoOverQuotaDialog</name>
    <message>
        <source>MEGAsync is currently disabled</source>
        <translation type="unfinished">MEGAsync is currently disabled</translation>
    </message>
    <message>
        <source>Your account has exceeded its allowed space quota.[A]Upgrade[/A]and keep enjoying secure, end-to-end encrypted storage.</source>
        <translation type="unfinished">Your account has exceeded its allowed space quota.[A]Upgrade[/A]and keep enjoying secure, end-to-end encrypted storage.</translation>
    </message>
    <message>
        <source>Upgrade your account</source>
        <translation type="unfinished">升級您的帳號</translation>
    </message>
    <message>
        <source>Usage: Data temporarily unavailable</source>
        <translation type="unfinished">用量:數據暫時無法取得</translation>
    </message>
    <message>
        <source>MEGA website</source>
        <translation type="unfinished">MEGA網站</translation>
    </message>
    <message>
        <source>%1 of %2</source>
        <translation type="unfinished">%1 / %2</translation>
    </message>
    <message>
        <source>Usage: %1</source>
        <translation type="unfinished">用量: %1</translation>
    </message>
</context>
<context>
    <name>Installer</name>
    <message>
        <source>Choose Users</source>
        <translatorcomment>Label to indicate for which users you want to install MEGAsync.</translatorcomment>
        <translation>選擇使用者</translation>
    </message>
    <message>
        <source>Choose for which users you want to install $(^NameDA).</source>
        <translatorcomment>Label to indicate for which users you want to install MEGAsync. Preserve $(^NameDA) code</translatorcomment>
        <translation>Choose for which users you want to install $(^NameDA).</translation>
    </message>
    <message>
        <source>Select whether you want to install $(^NameDA) for yourself only or for all users of this computer. $(^ClickNext)</source>
        <translatorcomment>Label to indicate the type of installation. Preserve $(^NameDA)  and $(^ClickNext) codes.</translatorcomment>
        <translation>Select whether you want to install $(^NameDA) for yourself only or for all users of this computer. $(^ClickNext)</translation>
    </message>
    <message>
        <source>Install for anyone using this computer</source>
        <translatorcomment>Label to indicate that MEGAsync installation on Windows is for anyone using this computer.</translatorcomment>
        <translation>為這台電腦的所有使用者安裝</translation>
    </message>
    <message>
        <source>Install just for me</source>
        <translatorcomment>Label to indicate that MEGAsync installation on Windows is just for the current user.</translatorcomment>
        <translation>僅為自己安裝</translation>
    </message>
</context>
<context>
    <name>MegaApplication</name>
    <message>
        <source>MEGAsync</source>
        <translation>MEGAsync</translation>
    </message>
    <message>
        <source>Thank you for testing MEGAsync.&lt;br&gt;This beta version is no longer current and has expired.&lt;br&gt;Please follow &lt;a href=&quot;https://twitter.com/MEGAprivacy&quot;&gt;@MEGAprivacy&lt;/a&gt; on Twitter for updates.</source>
        <translation type="obsolete">Thank you for testing MEGAsync.<br>This beta version is no longer current and has expired.<br>Please follow <a href=&quot;https://twitter.com/MEGAprivacy&quot;>@MEGAprivacy</a> on Twitter for updates.</translation>
    </message>
    <message>
        <source>Logging in</source>
        <translatorcomment>Label of tray icon showing a Logging in state. Keep capital letters.</translatorcomment>
        <translation>正在登入</translation>
    </message>
    <message>
        <source>MEGAsync is now running. Click here to open the status window.</source>
        <translatorcomment>Notification message that MEGAsync is actually running.</translatorcomment>
        <translation>MEGAsync is now running. Click here to open the status window.</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled
because the remote folder doesn&apos;t exist</source>
        <translation type="obsolete">Your sync &quot;%1&quot; has been disabledbecause the remote folder doesn&apos;t exist</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled
because the remote folder is in the rubbish bin</source>
        <translation type="obsolete">Your sync &quot;%1&quot; has been disabledbecause the remote folder is in the rubbish bin</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled
because the local folder doesn&apos;t exist</source>
        <translation type="obsolete">因本機資料夾不存在，您的同步 &quot;%1&quot; 已被停用。</translation>
    </message>
    <message>
        <source>Error: Invalid destination folder. The upload has been cancelled</source>
        <translatorcomment>Notification message launched when a problem occurs uploading files to a destination folder in MEGA.</translatorcomment>
        <translation>錯誤:目的地資料夾無效。上傳已被取消。</translation>
    </message>
    <message>
        <source>The folder (%1) wasn&apos;t uploaded because it&apos;s too large (this beta is limited to %2 folders or %3 files.</source>
        <translation type="obsolete">The folder (%1) wasn&apos;t uploaded because it&apos;s too large (this beta is limited to %2 folders or %3 files.</translation>
    </message>
    <message>
        <source>%1 folders weren&apos;t uploaded because they are too large (this beta is limited to %2 folders or %3 files.</source>
        <translation type="obsolete">%1 folders weren&apos;t uploaded because they are too large (this beta is limited to %2 folders or %3 files.</translation>
    </message>
    <message>
        <source>Synchronization will stop.
Deletions that occur while it is not running will not be propagated.

Exit anyway?</source>
        <translation type="obsolete">Synchronization will stop.Deletions that occur while it is not running will not be propagated.Exit anyway?</translation>
    </message>
    <message>
        <source>About MEGAsync</source>
        <translatorcomment>Title of the dialog that displays the version code of MEGAsync.</translatorcomment>
        <translation>關於MEGAsync</translation>
    </message>
    <message>
        <source>MEGAsync version code %1</source>
        <translatorcomment>Label to indicate the version code of MEGAsync installed. Preserve &quot;%1&quot; code because is used to indicate the version code at runtime.</translatorcomment>
        <translation>MEGAsync 版本 %1</translation>
    </message>
    <message>
        <source>The link has been copied to the clipboard</source>
        <translatorcomment>Notification message launched when a link to a file has been copied succesfully to the clipboard.</translatorcomment>
        <translation>此連結已被複製到剪貼簿</translation>
    </message>
    <message>
        <source>The links have been copied to the clipboard</source>
        <translatorcomment>Notification message launched when some links have been copied succesfully to the clipboard.</translatorcomment>
        <translation>此連結已被複製到剪貼簿</translation>
    </message>
    <message>
        <source>Logging in...</source>
        <translatorcomment>Notification message showing a Logging in state. Keep capital letters.</translatorcomment>
        <translation>Logging in...</translation>
    </message>
    <message>
        <source>Exit</source>
        <translatorcomment>Label to indicate the Exit option for the application (MAX 20 characters)</translatorcomment>
        <translation>離開</translation>
    </message>
    <message>
        <source>About</source>
        <translatorcomment>Label to indicate the About option for the application (MAX 20 characters)</translatorcomment>
        <translation>關於</translation>
    </message>
    <message>
        <source>Settings</source>
        <translatorcomment>Label to indicate the Settings option for the application (MAX 20 characters)</translatorcomment>
        <translation>設定</translation>
    </message>
    <message>
        <source>Pause</source>
        <translation type="obsolete">暫停</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation type="obsolete">恢復</translation>
    </message>
    <message>
        <source>Import links</source>
        <translatorcomment>Label to indicate the Import links option for the application (MAX 20 characters)</translatorcomment>
        <translation>匯入連結</translation>
    </message>
    <message>
        <source>Up to date</source>
        <translatorcomment>Label of tray icon to indicate that MEGAsync is up to date and there isn&apos;t any available update.</translatorcomment>
        <translation>最新的</translation>
    </message>
    <message>
        <source>Paused</source>
        <translatorcomment>Label of tray icon to indicate that MEGAsync is in a paused state.</translatorcomment>
        <translation>暫停</translation>
    </message>
    <message>
        <source>Scanning</source>
        <translatorcomment>Label of tray icon to indicate that MEGAsync is in a scanning state.</translatorcomment>
        <translation>掃描中</translation>
    </message>
    <message>
        <source>Syncing</source>
        <translatorcomment>Label of tray icon to indicate that MEGAsync is in a syncing state.</translatorcomment>
        <translation>同步中</translation>
    </message>
    <message>
        <source>Temporary transmission error: </source>
        <translatorcomment>Notification message launched when there is a temporal problem with a transfer. Keep colon.</translatorcomment>
        <translation>暫時性傳輸錯誤:</translation>
    </message>
    <message>
        <source>You have new or updated files in your account</source>
        <translatorcomment>Notification message launched when new or updated files have been added or modified to the current MEGA account.</translatorcomment>
        <translation>您的帳號內有新的或更新的檔案</translation>
    </message>
    <message>
        <source>MEGAsync has been updated</source>
        <translatorcomment>Notification message launched when an update has been succesfully applied.</translatorcomment>
        <translation>MEGAsync 已經更新</translation>
    </message>
    <message>
        <source>Waiting</source>
        <translatorcomment>Label of tray icon to indicate that MEGAsync is in a waiting state.</translatorcomment>
        <translation>請等待</translation>
    </message>
    <message>
        <source>The folder (%1) wasn&apos;t uploaded because it&apos;s extremely large. We do this check to prevent the uploading of entire boot volumes, which is inefficient and dangerous.</source>
        <translation type="obsolete">資料夾(%1)因過大而未被上傳。我們進行此項檢查來預防大量上傳，以免效率過差或危險。</translation>
    </message>
    <message>
        <source>%1 folders weren&apos;t uploaded because they are extremely large. We do this check to prevent the uploading of entire boot volumes, which is inefficient and dangerous.</source>
        <translation type="obsolete">%1資料夾因過大而未被上傳。我們進行此項檢查來預防大量上傳，以免效率過差或危險。</translation>
    </message>
    <message>
        <source>Update available!</source>
        <translatorcomment>Label of tray icon to indicate that there is an update available to download.</translatorcomment>
        <translation>取得更新版本!</translation>
    </message>
    <message>
        <source>An update will be applied during the next application restart</source>
        <translatorcomment>Notification message launched when an update is already downloaded but not applied yet. It will be applied during the next application restart.</translatorcomment>
        <translation>更新將於應用程式下次啟動時套用。</translation>
    </message>
    <message>
        <source>Installing update...</source>
        <translatorcomment>Notification message launched when an update is being installed.</translatorcomment>
        <translation>安裝更新...</translation>
    </message>
    <message>
        <source>Checking for updates...</source>
        <translatorcomment>Notification message launched when the user wants to check if there are any available update at the moment.</translatorcomment>
        <translation>檢查更新...</translation>
    </message>
    <message>
        <source>Install update</source>
        <translatorcomment>Label to indicate the user that there is an available update downloaded to be installed.</translatorcomment>
        <translation>安裝更新</translation>
    </message>
    <message>
        <source>A new version of MEGAsync is available! Click on this message to install it</source>
        <translatorcomment>Notification message launched to inform the user that there is an available update .</translatorcomment>
        <translation>已推出更新版的MEGAsync! 點擊此訊息即可安裝</translation>
    </message>
    <message>
        <source>There was a problem installing the update. Please try again later or download the last version from:
https://mega.co.nz/#sync</source>
        <translatorcomment>Notification message launched when a problem occurs during the installation of an update. Keep &quot;\n&quot; codes.</translatorcomment>
        <translation>There was a problem installing the update. Please try again later or download the last version from:https://mega.co.nz/#sync</translation>
    </message>
    <message>
        <source>Thank you for your collaboration!</source>
        <translatorcomment>Message displayed to thank when a user send a crash report report to MEGA.</translatorcomment>
        <translation>謝謝您的合作!</translation>
    </message>
    <message>
        <source>Update available. Downloading...</source>
        <translatorcomment>Notification message launched when an update for MEGAsync is being downloaded.</translatorcomment>
        <translation>有可用的更新。下載中...</translation>
    </message>
    <message>
        <source>No update available at this time</source>
        <translatorcomment>Notification message launched when a user is cheking if there are any available updates.</translatorcomment>
        <translation>No update available at this time</translation>
    </message>
    <message>
        <source>Error</source>
        <translatorcomment>Label to indicate an error. Keep capital letter.</translatorcomment>
        <translation>錯誤</translation>
    </message>
    <message>
        <source>Synchronization will stop.

Exit anyway?</source>
        <translatorcomment>Message displayed when a user is exiting the application while there are any active synchronization. Keep &quot;\n&quot; codes and capital letters.</translatorcomment>
        <translation>Synchronization will stop.Exit anyway?</translation>
    </message>
    <message>
        <source>Starting</source>
        <translatorcomment>Label of tray icon to indicate that MEGAsync is starting.</translatorcomment>
        <translation>Starting</translation>
    </message>
    <message>
        <source>Unable to get the filesystem.
Please, try again. If the problem persists please contact bug@mega.co.nz</source>
        <translatorcomment>Message displayed when an error occurs while fetching nodes from the server.</translatorcomment>
        <translation>Unable to get the filesystem.Please, try again. If the problem persists please contact bug@mega.co.nz</translation>
    </message>
    <message>
        <source>Upload files/folders</source>
        <translation type="obsolete">上傳檔案/資料夾</translation>
    </message>
    <message>
        <source>MEGAsync is now running. Click the system tray icon to open the status window.</source>
        <translatorcomment>Notification message launched when an user logged in succesfully. String as short as possible.</translatorcomment>
        <translation>MEGAsync執行中。點選系統圖示可開啟狀態視窗。</translation>
    </message>
    <message>
        <source>A new version of MEGAsync is available!</source>
        <translatorcomment>Notification message launched when an update is available.</translatorcomment>
        <translation>已有新版MEGAsync可用!</translation>
    </message>
    <message>
        <source>MEGAsync is now running. Click the menu bar icon to open the status window.</source>
        <translatorcomment>Notification message launched when an user logged in succesfully. String as short as possible.</translatorcomment>
        <translation>MEGAsync is now running. Click the menu bar icon to open the status window.</translation>
    </message>
    <message>
        <source>Quit</source>
        <translatorcomment>Label displayed to let the user quit the application. Max 20 characters. Keep capital letter.</translatorcomment>
        <translation>Quit</translation>
    </message>
    <message>
        <source>Preferences</source>
        <translatorcomment>Label and title of the preferences dialog. MAX 20 characters. Keep capital letter.</translatorcomment>
        <translation>Preferences</translation>
    </message>
    <message>
        <source>Upload to MEGA</source>
        <translatorcomment>Label and title of the dialog displayed when a user wants to upload file/folder to MEGA. MAX 20 characters. Keep capital letters.</translatorcomment>
        <translation>上傳至MEGA</translation>
    </message>
    <message>
        <source>Show status</source>
        <translatorcomment>Label displayed to let the user displays the Information dialog of MEGAsync. MAX 20 characters. Keep capital letter.</translatorcomment>
        <translation>顯示狀態</translation>
    </message>
    <message>
        <source>Your config is corrupt, please start over</source>
        <translatorcomment>Message displayed when an error occurs loading configurations from Settings file.</translatorcomment>
        <translation>您的config已損壞，請重新開始。</translation>
    </message>
    <message>
        <source>Download from MEGA</source>
        <translatorcomment>Label and title of the dialog displayed when a user wants to download file/folder from MEGA. MAX 20 characters. Keep capital letters.</translatorcomment>
        <translation>從MEGA下載</translation>
    </message>
    <message>
        <source>Error getting link: </source>
        <translatorcomment>Notification message launched when an error occurs getting a public link for a file/folder. Keep colon.</translatorcomment>
        <translation>生成鏈接出錯：</translation>
    </message>
    <message>
        <source>MEGAsync is unable to connect. Please check your Internet connectivity and local firewall configuration. Note that most antivirus software includes a firewall.</source>
        <translatorcomment>Notification message launched when a connectivity problem occurs.</translatorcomment>
        <translation type="unfinished">MEGAsync is unable to connect. Please check your Internet connectivity and local firewall configuration. Note that most antivirus software includes a firewall.</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled because the remote folder doesn&apos;t exist</source>
        <translatorcomment>Notification message launched checking  remote synchronization folders. Keep &quot;%1&quot; code because is filled with the name of folder at runtime.</translatorcomment>
        <translation type="unfinished">因遠端資料夾不存在，您的同步 &quot;%1&quot; 已被停用。</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled because the local folder doesn&apos;t exist</source>
        <translatorcomment>Notification message launched checking  local synchronization folders. Keep &quot;%1&quot; code because is filled with the name of folder at runtime.</translatorcomment>
        <translation type="unfinished">Your sync &quot;%1&quot; has been disabled because the local folder doesn&apos;t exist</translation>
    </message>
    <message>
        <source>Your account has been blocked. Please contact support@mega.co.nz</source>
        <translatorcomment>Message displayed when an error occurs (BLOCK ACCOUNT) during a login operation.</translatorcomment>
        <translation type="unfinished">Your account has been blocked. Please contact support@mega.co.nz</translation>
    </message>
    <message>
        <source>Login error: %1</source>
        <translatorcomment>Message displayed during a login operation. Keep &quot;%1&quot; code because it will be fill with the error message.</translatorcomment>
        <translation type="unfinished">Login error: %1</translation>
    </message>
    <message>
        <source>You have been logged out on this computer from another location</source>
        <translatorcomment>Message displayed when the current account has been logged out from other computer/website.</translatorcomment>
        <translation type="unfinished">You have been logged out on this computer from another location</translation>
    </message>
    <message>
        <source>You have been logged out because of this error: %1</source>
        <translatorcomment>Message displayed when the current account has been logged due to an error. Keep &quot;%1&quot; code because it will be filled with the error message.</translatorcomment>
        <translation type="unfinished">You have been logged out because of this error: %1</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled because the remote folder is in the rubbish bin</source>
        <translatorcomment>Notification message launched when a sync is disabled due to the remote folder has been deleted. Keep &quot;%1&quot; code because it will be filled with the folder name.</translatorcomment>
        <translation type="unfinished">因遠端資料夾已於垃圾夾內，您的同步 &quot;%1&quot; 已被停用。</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled because the local folder has changed</source>
        <translatorcomment>Notification message launched when a sync is disabled due to the local folder has changed(moved/deleted/...). Keep &quot;%1&quot; code because it will be filled with the folder name.</translatorcomment>
        <translation type="unfinished">Your sync &quot;%1&quot; has been disabled because the local folder has changed</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled. The remote folder (or part of it) doesn&apos;t have full access</source>
        <translatorcomment>Notification message launched when a sync is disabled due to the access problems. Keep &quot;%1&quot; code because it will be filled with the folder name.</translatorcomment>
        <translation type="unfinished">Your sync &quot;%1&quot; has been disabled. The remote folder (or part of it) doesn&apos;t have full access</translation>
    </message>
    <message>
        <source>Over quota</source>
        <translation type="unfinished">超過額度</translation>
    </message>
    <message>
        <source>Your sync &quot;%1&quot; has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.</source>
        <translation type="unfinished">Your sync &quot;%1&quot; has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.</translation>
    </message>
    <message>
        <source>Logout</source>
        <translation type="unfinished">登出</translation>
    </message>
    <message>
        <source>Transfer failed:</source>
        <translation type="unfinished">Transfer failed:</translation>
    </message>
    <message>
        <source>Error getting link information</source>
        <translation type="unfinished">Error getting link information</translation>
    </message>
</context>
<context>
    <name>MegaError</name>
    <message>
        <source>No error</source>
        <translatorcomment>Label to show that an SDK operation has been complete successfully.</translatorcomment>
        <translation>無誤</translation>
    </message>
    <message>
        <source>Internal error</source>
        <translatorcomment>Label to show that an Internal error occurs during a SDK operation.</translatorcomment>
        <translation>內部錯誤</translation>
    </message>
    <message>
        <source>Invalid argument</source>
        <translatorcomment>Label to show that an error of Invalid argument occurs during a SDK operation.</translatorcomment>
        <translation>無效參數</translation>
    </message>
    <message>
        <source>Request failed, retrying</source>
        <translatorcomment>Label to show that a request error occurs during a SDK operation.</translatorcomment>
        <translation>請求失敗，重試中</translation>
    </message>
    <message>
        <source>Rate limit exceeded</source>
        <translatorcomment>Label to show that the rate limit has been reached during a SDK operation.</translatorcomment>
        <translation>超過速度限制</translation>
    </message>
    <message>
        <source>Failed permanently</source>
        <translatorcomment>Label to show that a SDK operation has failed permanently.</translatorcomment>
        <translation>已失敗</translation>
    </message>
    <message>
        <source>Too many concurrent connections or transfers</source>
        <translatorcomment>Label to show that an error for multiple concurrent connections or transfers occurs during a SDK operation.</translatorcomment>
        <translation>過多併行連結或傳輸</translation>
    </message>
    <message>
        <source>Out of range</source>
        <translatorcomment>Label to show that an error of Out of range occurs during a SDK operation.</translatorcomment>
        <translation>超出範圍</translation>
    </message>
    <message>
        <source>Expired</source>
        <translatorcomment>Label to show that an error related with expiration occurs during a SDK operation.</translatorcomment>
        <translation>過期</translation>
    </message>
    <message>
        <source>Not found</source>
        <translatorcomment>Label to show that an error related with a resource Not found occurs during a SDK operation.</translatorcomment>
        <translation>未找到</translation>
    </message>
    <message>
        <source>Circular linkage detected</source>
        <translatorcomment>Label to show that an error related with a circular linkage occurs during a SDK operation.</translatorcomment>
        <translation>Circular linkage detected</translation>
    </message>
    <message>
        <source>Access denied</source>
        <translatorcomment>Label to show that an error related with an denied access occurs during a SDK operation.</translatorcomment>
        <translation>進入遭拒</translation>
    </message>
    <message>
        <source>Already exists</source>
        <translatorcomment>Label to show that an error related with an existent resource occurs during a SDK operation.</translatorcomment>
        <translation>已存在</translation>
    </message>
    <message>
        <source>Incomplete</source>
        <translatorcomment>Label to show that an error related with an Incomplete SDK operation.</translatorcomment>
        <translation>未完成</translation>
    </message>
    <message>
        <source>Invalid key/Decryption error</source>
        <translatorcomment>Label to show that an error related with the decryption process of a node occurs during a SDK operation.</translatorcomment>
        <translation>無效金鑰/解密錯誤</translation>
    </message>
    <message>
        <source>Bad session ID</source>
        <translatorcomment>Label to show that an error related with a bad session ID occurs during a SDK operation.</translatorcomment>
        <translation>惡意session ID</translation>
    </message>
    <message>
        <source>Blocked</source>
        <translatorcomment>Label to show that an error related with a blocked account occurs during a SDK operation.</translatorcomment>
        <translation>封鎖</translation>
    </message>
    <message>
        <source>Over quota</source>
        <translatorcomment>Label to show that an error related with an over quota occurs during a SDK operation.</translatorcomment>
        <translation>超過額度</translation>
    </message>
    <message>
        <source>Temporarily not available</source>
        <translatorcomment>Label to show that an error related with a temporary problem occurs during a SDK operation.</translatorcomment>
        <translation>暫時無法取得</translation>
    </message>
    <message>
        <source>Connection overflow</source>
        <translatorcomment>Label to show that an error related with too many connections occurs during a SDK operation.</translatorcomment>
        <translation>連接過量</translation>
    </message>
    <message>
        <source>Write error</source>
        <translatorcomment>Label to show that an error related with an write error occurs during a SDK operation.</translatorcomment>
        <translation>寫入錯誤</translation>
    </message>
    <message>
        <source>Read error</source>
        <translatorcomment>Label to show that an error related with an read error occurs during a SDK operation.</translatorcomment>
        <translation>讀取錯誤</translation>
    </message>
    <message>
        <source>Invalid application key</source>
        <translatorcomment>Label to show that an error related with an invalid or missing application key occurs during a SDK operation.</translatorcomment>
        <translation>無效應用程式金鑰</translation>
    </message>
    <message>
        <source>Unknown error</source>
        <translatorcomment>Label to show that an error related with an unknown error occurs during a SDK operation.</translatorcomment>
        <translation>未知的錯誤</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="obsolete">錯誤</translation>
    </message>
</context>
<context>
    <name>MegaUploader</name>
    <message>
        <source>Warning</source>
        <translatorcomment>Label displayed for a Warning message. Keep capital letter.</translatorcomment>
        <translation type="obsolete">警告</translation>
    </message>
    <message>
        <source>The destination folder is synced and you already have a file 
inside it with the same name (%1).
If you continue the upload, the previous file will be overwritten.
Are you sure?</source>
        <translatorcomment>Message displayed when a user tries to upload a file to a synced folder wich already contains it  Ask for confirmation. Keep  (%1) code because it will be filled with name of the file. String as short as possible.</translatorcomment>
        <translation type="obsolete">The destination folder is synced and you already have a file inside it with the same name (%1).If you continue the upload, the previous file will be overwritten.Are you sure?</translation>
    </message>
</context>
<context>
    <name>MessageBox</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">警告</translation>
    </message>
    <message>
        <source>Do not ask me again</source>
        <translation type="unfinished">Do not ask me again</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="unfinished">取消</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>The destination folder is synced and you already have a file inside it with the same name. If you continue the upload, the previous file will be overwritten. Are you sure?</source>
        <translation type="unfinished">The destination folder is synced and you already have a file inside it with the same name. If you continue the upload, the previous file will be overwritten. Are you sure?</translation>
    </message>
</context>
<context>
    <name>NodeSelector</name>
    <message>
        <source>Folder Selection</source>
        <translatorcomment>Label to indicate the user the selection of folders for a synchronization (String short as possible)</translatorcomment>
        <translation>選擇資料夾</translation>
    </message>
    <message>
        <source>Select a MEGA folder:</source>
        <translatorcomment>Label to indicate the user to select a MEGA folder for a synchronization (String short as possible). Keep capital letters.</translatorcomment>
        <translation>選擇MEGA資料夾:</translation>
    </message>
    <message>
        <source>Retrieving folders...</source>
        <translatorcomment>Label to indicate the user that remote folders are being retrieving to be displayed (String short as possible)</translatorcomment>
        <translation>Retrieving folders...</translation>
    </message>
    <message>
        <source>New folder</source>
        <translatorcomment>Button label to create a New folder at your MEGA cloud drive. Keep capital letters. String as short as possible.</translatorcomment>
        <translation>新資料夾</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Cloud Drive</source>
        <translatorcomment>Label to indicate the root folder of your MEGA cloud drive.</translatorcomment>
        <translation>雲端硬碟</translation>
    </message>
    <message>
        <source>Enter the new folder name:</source>
        <translatorcomment>Label to indicate the user for the name of the new folder wich will be created at the Cloud Drive.</translatorcomment>
        <translation>輸入新資料夾名稱:</translation>
    </message>
    <message>
        <source>Error</source>
        <translatorcomment>Label to indicate an Error</translatorcomment>
        <translation>錯誤</translation>
    </message>
    <message>
        <source>The root folder can&apos;t be synced.
Please, select a subfolder.</source>
        <translatorcomment>Message displayed when a user is creating incompatible synchronizations.</translatorcomment>
        <translation>The root folder can&apos;t be synced.Please, select a subfolder.</translation>
    </message>
    <message>
        <source>Warning</source>
        <translatorcomment>Label to indicate a Warning message.</translatorcomment>
        <translation>警告</translation>
    </message>
    <message>
        <source>You have %1 in this folder.
Are you sure you want to sync it?</source>
        <translation type="obsolete">You have %1 in this folder.Are you sure you want to sync it?</translation>
    </message>
    <message>
        <source>Invalid folder for synchronization.
Please, ensure that you don&apos;t use characters like &apos;\&apos; &apos;/&apos; or &apos;:&apos; in your folder names.</source>
        <translatorcomment>Message displayed when a user is trying to create a synchronization using not allowed characthers. Keep  &apos;\\&apos; &apos;/&apos; and &apos;:&apos; codes.</translatorcomment>
        <translation>Invalid folder for synchronization.Please, ensure that you don&apos;t use characters like &apos;&apos; &apos;/&apos; or &apos;:&apos; in your folder names.</translation>
    </message>
    <message>
        <source>Always upload to this destination</source>
        <translatorcomment>Label to inform the user upload files/folder to a default destination. with a checkbox.</translatorcomment>
        <translation>每次上傳都以此為目的地資料夾</translation>
    </message>
    <message>
        <source>You need Read &amp; Write or Full access rights to be able to upload to the selected folder.</source>
        <translation type="unfinished">You need Read & Write or Full access rights to be able to upload to the selected folder.</translation>
    </message>
    <message>
        <source>You need Full access right to be able to sync the selected folder.</source>
        <translation type="unfinished">You need Full access right to be able to sync the selected folder.</translation>
    </message>
</context>
<context>
    <name>PasteMegaLinksDialog</name>
    <message>
        <source>Import links</source>
        <translatorcomment>Label and Title of the dialog displayed when a user is trying to import public MEGA links.(MAX 20 characters)</translatorcomment>
        <translation>匯入連結</translation>
    </message>
    <message>
        <source>Enter one or multiple MEGA file links</source>
        <translatorcomment>Label to indicate the user to write down the links to be imported. String as short as possible.</translatorcomment>
        <translation>輸入一個或多個MEGA檔案連結</translation>
    </message>
    <message>
        <source>Submit</source>
        <translatorcomment>Label for submit button. Keep capital letter.</translatorcomment>
        <translation>提交</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button. Keep capital letter.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Warning</source>
        <translatorcomment>Label displayed for a Warning message. Keep capital letter.</translatorcomment>
        <translation>警告</translation>
    </message>
    <message>
        <source>Enter one or more MEGA file links</source>
        <translatorcomment>Message displayed when a user tries to import some links but the field is empty.</translatorcomment>
        <translation>輸入一個或多個MEGA檔案連結</translation>
    </message>
    <message>
        <source>No valid MEGA links found. (Folder links aren&apos;t yet supported)</source>
        <translatorcomment>Message displayed when a user tries to import some invalid links or links to folders.</translatorcomment>
        <translation>No valid MEGA links found. (Folder links aren&apos;t yet supported)</translation>
    </message>
</context>
<context>
    <name>QDialogButtonBox</name>
    <message>
        <source>&amp;Yes</source>
        <translatorcomment>Label for confirm button. Keep capital letter.</translatorcomment>
        <translation>&Yes</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translatorcomment>Label for No button. Keep capital letter.</translatorcomment>
        <translation>&No</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translatorcomment>Label for accept button. Keep capital letter.</translatorcomment>
        <translation>&OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translatorcomment>Label for cancel button. Keep capital letter.</translatorcomment>
        <translation>&Cancel</translation>
    </message>
</context>
<context>
    <name>RecentFile</name>
    <message>
        <source>%1 hours ago</source>
        <translatorcomment>Label to inform the user how many hours ago was updated the indicated file. Keep %1 code because it will be filled with the name of the updated file.</translatorcomment>
        <translation>%1小時以前</translation>
    </message>
    <message>
        <source>Get MEGA link</source>
        <translatorcomment>Button tooltip to generate a public link for a specific file. Keep capital letters.</translatorcomment>
        <translation>取得MEGA連結</translation>
    </message>
    <message>
        <source>just now</source>
        <translatorcomment>Label to inform the user that the indicated file has been updated just now.</translatorcomment>
        <translation>剛剛</translation>
    </message>
    <message>
        <source>%1 seconds ago</source>
        <translatorcomment>Label to inform the user how many second ago was updated the indicated file. Keep %1 code because it will be filled with the name of the updated file.</translatorcomment>
        <translation>%1 秒前</translation>
    </message>
    <message>
        <source>1 minute ago</source>
        <translatorcomment>Label to inform the user that the indicated file has been updated one minute ago.</translatorcomment>
        <translation>1 分鐘前</translation>
    </message>
    <message>
        <source>%1 minutes ago</source>
        <translatorcomment>Label to inform the user how many minutes ago was updated the indicated file. Keep %1 code because it will be filled with the name of the updated file.</translatorcomment>
        <translation>%1 分鐘前</translation>
    </message>
    <message>
        <source>1 hour ago</source>
        <translatorcomment>Label to inform the user that the indicated file has been updated one hour ago.</translatorcomment>
        <translation>1 小時前</translation>
    </message>
    <message>
        <source>1 day ago</source>
        <translatorcomment>Label to inform the user that the indicated file has been updated one day ago.</translatorcomment>
        <translation>1 天前</translation>
    </message>
    <message>
        <source>%1 days ago</source>
        <translatorcomment>Label to inform the user how many days ago was updated the indicated file. Keep %1 code because it will be filled with the name of the updated file.</translatorcomment>
        <translation>%1 天前</translation>
    </message>
    <message>
        <source>1 month ago</source>
        <translatorcomment>Label to inform the user that the indicated file has been updated one month ago.</translatorcomment>
        <translation>1 個月前</translation>
    </message>
    <message>
        <source>%1 months ago</source>
        <translatorcomment>Label to inform the user how many months ago was updated the indicated file. Keep %1 code because it will be filled with the name of the updated file.</translatorcomment>
        <translation>%1 個月前</translation>
    </message>
    <message>
        <source>1 year ago</source>
        <translatorcomment>Label to inform the user that the indicated file has been updated one year ago.</translatorcomment>
        <translation>1 年前</translation>
    </message>
    <message>
        <source>%1 years ago</source>
        <translatorcomment>Label to inform the user how many years ago was updated the indicated file. Keep %1 code because it will be filled with the name of the updated file.</translatorcomment>
        <translation>%1 年前</translation>
    </message>
    <message>
        <source>Open</source>
        <translatorcomment>Label to let the user open the selected file.String as short as possible.</translatorcomment>
        <translation>開啟</translation>
    </message>
    <message>
        <source>Show in folder</source>
        <translatorcomment>Label to let the user open the selected file using the specific file browser.String as short as possible.</translatorcomment>
        <translation>顯示資料夾</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <source>Settings - MEGAsync</source>
        <translatorcomment>Title of the MEGAsync Settings dialog. Keep capital letters.</translatorcomment>
        <translation>Settings - MEGAsync</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="obsolete">一班</translation>
    </message>
    <message>
        <source>Account</source>
        <translatorcomment>Tab label of Account dialog. Max 15 characters.</translatorcomment>
        <translation>帳號</translation>
    </message>
    <message>
        <source>Syncs</source>
        <translatorcomment>Tab label of Syncs dialog. Max 15 characters.</translatorcomment>
        <translation>同步</translation>
    </message>
    <message>
        <source>Bandwidth</source>
        <translatorcomment>Tab label of Bandwidth dialog. Max 15 characters.</translatorcomment>
        <translation>頻寬</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translatorcomment>Tab label of Advanced settings dialog. Max 15 characters.</translatorcomment>
        <translation>進階</translation>
    </message>
    <message>
        <source>Help</source>
        <translatorcomment>Button label for Help.</translatorcomment>
        <translation>支援</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Apply</source>
        <translatorcomment>Label for apply settings button.</translatorcomment>
        <translation>套用</translation>
    </message>
    <message>
        <source>Show notifications</source>
        <translatorcomment>Label to let the user enable desktop notifications with a checkbox.</translatorcomment>
        <translation>顯示通知</translation>
    </message>
    <message>
        <source>Start on startup</source>
        <translatorcomment>Label to let the user start MEGAsync on startup of the operating system with a checkbox.</translatorcomment>
        <translation>自動開啟</translation>
    </message>
    <message>
        <source>Update automatically</source>
        <translatorcomment>Label to let the user enable automatic updates with a checkbox.</translatorcomment>
        <translation>自動更新</translation>
    </message>
    <message>
        <source>Language</source>
        <translatorcomment>Label to let the user select the desired language for the application.</translatorcomment>
        <translation>語言</translation>
    </message>
    <message>
        <source>English</source>
        <translatorcomment>Label for English language.</translatorcomment>
        <translation>英文</translation>
    </message>
    <message>
        <source>Details</source>
        <translatorcomment>Button label to show the account usage details for the current user of the application. Strins as short as possible.</translatorcomment>
        <translation>內容</translation>
    </message>
    <message>
        <source>FREE</source>
        <translatorcomment>Label to indicate that the current user has a FREE account.</translatorcomment>
        <translation>免費</translation>
    </message>
    <message>
        <source>Logout</source>
        <translatorcomment>Button label to logout of the application. Strins as short as possible.</translatorcomment>
        <translation>登出</translation>
    </message>
    <message>
        <source>Storage space</source>
        <translatorcomment>Label to indicate the actual storage space used.</translatorcomment>
        <translation>儲存空間</translation>
    </message>
    <message>
        <source>Upgrade</source>
        <translatorcomment>Button label to let the user upgrade the account. String as short as possible.</translatorcomment>
        <translation>升級</translation>
    </message>
    <message>
        <source>Synced folders</source>
        <translation type="obsolete">同步資料夾</translation>
    </message>
    <message>
        <source>Delete</source>
        <translatorcomment>Button lable to delete a synchronization. String as short as possible.</translatorcomment>
        <translation>刪除</translation>
    </message>
    <message>
        <source>Add</source>
        <translatorcomment>Button lable to add a synchronization. String as short as possible.</translatorcomment>
        <translation>新增</translation>
    </message>
    <message>
        <source>Local Folder</source>
        <translatorcomment>Label to indicate the local folder for a synchronization. String as short as possible.</translatorcomment>
        <translation>本機資料夾</translation>
    </message>
    <message>
        <source>MEGA folder</source>
        <translatorcomment>Label to indicate the MEGA folder for a synchronization. String as short as possible.</translatorcomment>
        <translation>MEGA資料夾</translation>
    </message>
    <message>
        <source>Upload rate limit</source>
        <translatorcomment>Label to indicate the actual bandwidth limit for uploads.</translatorcomment>
        <translation>上傳速度限制</translation>
    </message>
    <message>
        <source>Don&apos;t limit</source>
        <translatorcomment>Label to indicate that there is no rate upload limit.</translatorcomment>
        <translation>不限</translation>
    </message>
    <message>
        <source>Limit to:</source>
        <translatorcomment>Label to indicate that there is rate upload limit. Keep colon.</translatorcomment>
        <translation>限於:</translation>
    </message>
    <message>
        <source>KB</source>
        <translatorcomment>Label to indicate Kilo byte upload limit.</translatorcomment>
        <translation>KB</translation>
    </message>
    <message>
        <source>Auto</source>
        <translatorcomment>Label to indicate that management of rate upload limit is automatic.</translatorcomment>
        <translation>自動</translation>
    </message>
    <message>
        <source>(about 90% of the available bandwidth)</source>
        <translatorcomment>Label to indicate that management of rate upload limit is automatic is about the 90% of availbale bandwidth. Keep parenthesis.</translatorcomment>
        <translation>(about 90% of the available bandwidth)</translation>
    </message>
    <message>
        <source>Bandwidth quota</source>
        <translatorcomment>Label to indicatte the actual use of Bandwidth quota.</translatorcomment>
        <translation>頻寬額度</translation>
    </message>
    <message>
        <source>Upload limits are per upload server and are applied when starting new uploads</source>
        <translation type="obsolete">上傳限制是指每次上傳伺服器或開始新的上傳時啟用。</translation>
    </message>
    <message>
        <source>Folder for uploads from Windows Explorer</source>
        <translation type="obsolete">自Windows Explorer上傳的資料夾</translation>
    </message>
    <message>
        <source>Excluded file names</source>
        <translatorcomment>Label to indicate the excluded file name for synchronizations. Keep capital letters.</translatorcomment>
        <translation>不包含檔名</translation>
    </message>
    <message>
        <source>Proxy Settings</source>
        <translatorcomment>Label to indicate the dialog of Proxy Settings. Keep capital letters.</translatorcomment>
        <translation>Proxy設定</translation>
    </message>
    <message>
        <source>No proxy</source>
        <translatorcomment>Label to indicate MEGAsync not to use any proxy. String as short as possible.</translatorcomment>
        <translation>無Proxy</translation>
    </message>
    <message>
        <source>Auto-detect</source>
        <translatorcomment>Label to indicate auto detect. Keep capital letters.</translatorcomment>
        <translation>自動偵測</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translatorcomment>Tab label of Proxy dialog settings. Max 15 characters.</translatorcomment>
        <translation>Proxy</translation>
    </message>
    <message>
        <source>Proxy type:</source>
        <translatorcomment>Label to indicate the proxy type to be used. String as short as possible.</translatorcomment>
        <translation>Proxy類型:</translation>
    </message>
    <message>
        <source>Server:</source>
        <translatorcomment>Label to indicate the server IP to be used. String as short as possible.</translatorcomment>
        <translation>伺服器:</translation>
    </message>
    <message>
        <source>:</source>
        <translatorcomment>Label to indicate colon to separate IP and Port to be used.</translatorcomment>
        <translation>:</translation>
    </message>
    <message>
        <source>Proxy server requires a password</source>
        <translatorcomment>Label to indicate if the proxy used requires a password. String as short as possible.</translatorcomment>
        <translation>Proxy伺服器須輸入密碼</translation>
    </message>
    <message>
        <source>Username:</source>
        <translatorcomment>Label to indicate the username of the proxy. String as short as possible.</translatorcomment>
        <translation>使用者名稱:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translatorcomment>Label to indicate the password of the proxy. String as short as possible.</translatorcomment>
        <translation>密碼:</translation>
    </message>
    <message>
        <source>Data temporarily unavailable</source>
        <translatorcomment>Label to indicate that the usage data is temporarily unavailable. String as short as possible.</translatorcomment>
        <translation>資料暫時無法取得</translation>
    </message>
    <message>
        <source>%1 (%2%) of %3 used</source>
        <translatorcomment>Label to indicate the user the amount and percentage of used space and total space available. Keep %1 (%2%) and %3 codes because they will be filled with the required amounts of storage space.</translatorcomment>
        <translation>%1 (%2%) / %3 已使用</translation>
    </message>
    <message>
        <source>PRO I</source>
        <translatorcomment>Label to indicate that the current user has a PRO I account.</translatorcomment>
        <translation>Pro 1</translation>
    </message>
    <message>
        <source>PRO II</source>
        <translatorcomment>Label to indicate that the current user has a PRO II account.</translatorcomment>
        <translation>PRO II</translation>
    </message>
    <message>
        <source>PRO III</source>
        <translatorcomment>Label to indicate that the current user has a PRO III account.</translatorcomment>
        <translation>PRO III</translation>
    </message>
    <message>
        <source>/MEGAsync Uploads</source>
        <translatorcomment>Label to indicate the default path for MEGAsync uploads.</translatorcomment>
        <translation>/MEGAsync Uploads</translation>
    </message>
    <message>
        <source>Warning</source>
        <translatorcomment>Label to indicate a Warning message. Keep capital letter.</translatorcomment>
        <translation>警告</translation>
    </message>
    <message>
        <source>You are already syncing your entire Cloud Drive.</source>
        <translation type="obsolete">You are already syncing your entire Cloud Drive.</translation>
    </message>
    <message>
        <source>Synchronization will stop working.</source>
        <translatorcomment>Label to indicate that the synchronizations will stop if the user logout .</translatorcomment>
        <translation>同步動作將停止。</translation>
    </message>
    <message>
        <source>Are you sure?</source>
        <translatorcomment>Label to ask for confirmation to the user.</translatorcomment>
        <translation>您確定嗎?</translation>
    </message>
    <message>
        <source>Excluded name</source>
        <translatorcomment>Title of the dialog to add new excluded file name for the synchronizations.</translatorcomment>
        <translation>排除檔名</translation>
    </message>
    <message>
        <source>Enter a name to exclude from synchronization.
(wildcards * and ? are allowed):</source>
        <translatorcomment>Label to let the user add a new excluded file name. Keep colon.</translatorcomment>
        <translation>Enter a name to exclude from synchronization.(wildcards * and ? are allowed):</translation>
    </message>
    <message>
        <source>Error</source>
        <translatorcomment>Label to indicate an Error message. Keep capital letter.</translatorcomment>
        <translation>錯誤</translation>
    </message>
    <message>
        <source>Transfers</source>
        <translatorcomment>Tab label of Transfers dialog. Max 15 characters.</translatorcomment>
        <translation>transfers</translation>
    </message>
    <message>
        <source>The new excluded file names will be taken into account
when the application starts again.</source>
        <translation type="obsolete">The new excluded file names will be taken into accountwhen the application starts again.</translation>
    </message>
    <message>
        <source>Cache</source>
        <translation type="obsolete">暫存</translation>
    </message>
    <message>
        <source>Current cache size: %1</source>
        <translatorcomment>Label to indicate the user the total amount of space used by cache. Keep %1 code because it will be filled with the size amount used.</translatorcomment>
        <translation type="obsolete">目前暫存大小: %1</translation>
    </message>
    <message>
        <source>Clear</source>
        <translatorcomment>Label to let the user clear the cache. Keep capital letter.</translatorcomment>
        <translation>清除</translation>
    </message>
    <message>
        <source>HTTP</source>
        <translatorcomment>Label to indicate a HTTP proxy.</translatorcomment>
        <translation>HTTP</translation>
    </message>
    <message>
        <source>Your proxy settings are invalid or the proxy doesn&apos;t respond</source>
        <translatorcomment>Message displayed when an error occours testing proxy settings.</translatorcomment>
        <translation>您的proxy伺服器設定無效或proxy伺服器未回應。</translation>
    </message>
    <message>
        <source>Please wait...</source>
        <translatorcomment>Label to indicate the user that please wait.</translatorcomment>
        <translation>請等待...</translation>
    </message>
    <message>
        <source>Check for updates</source>
        <translatorcomment>Button label to let the user check for new application updates.</translatorcomment>
        <translation>檢查更新</translation>
    </message>
    <message>
        <source>Selective sync active</source>
        <translation type="obsolete">選擇性啟用同步</translation>
    </message>
    <message>
        <source>Enable full account sync</source>
        <translation type="obsolete">啟用全帳戶同步</translation>
    </message>
    <message>
        <source>Enabling full account sync will disable all your current syncs</source>
        <translation type="obsolete">啟用全帳戶同步將停用您目前的選擇性同步狀態</translation>
    </message>
    <message>
        <source>Full account sync active</source>
        <translation type="obsolete">全帳戶同步已啟用</translation>
    </message>
    <message>
        <source>Disabling full account sync will allow you to set up selective folder syncing</source>
        <translation type="obsolete">停用全帳戶同步後您將可設定選擇性同步</translation>
    </message>
    <message>
        <source>Disable full account sync</source>
        <translation type="obsolete">停用全帳戶同步</translation>
    </message>
    <message>
        <source>Other</source>
        <translation type="obsolete">其他</translation>
    </message>
    <message>
        <source>Disable overlay icons</source>
        <translatorcomment>Label to let the user disable overlay icon for the specific File browser.</translatorcomment>
        <translation>停用覆蓋按鈕</translation>
    </message>
    <message>
        <source>Force a full scan</source>
        <translatorcomment>Button label to let the user force a full scan of his synced folders.</translatorcomment>
        <translation>Force a full scan</translation>
    </message>
    <message>
        <source>Full scan</source>
        <translatorcomment>Title of the message dialog of Full scan operation.</translatorcomment>
        <translation>完整掃描</translation>
    </message>
    <message>
        <source>MEGAsync will perform a full scan of your synced folders
when it starts.

Do you want to restart MEGAsync now?</source>
        <translation type="obsolete">MEGAsync will perform a full scan of your synced folderswhen it starts.Do you want to restart MEGAsync now?</translation>
    </message>
    <message>
        <source>Install update</source>
        <translatorcomment>Label to indicate the user that there is an available update downloaded to be installed.</translatorcomment>
        <translation>安裝更新</translation>
    </message>
    <message>
        <source>Folder for uploads from this computer</source>
        <translation type="obsolete">由這台電腦上傳的檔案資料夾</translation>
    </message>
    <message>
        <source>Preferences - MEGAsync</source>
        <translatorcomment>Title label of the settings dialog.</translatorcomment>
        <translation>Preferences - MEGAsync</translation>
    </message>
    <message>
        <source>Open at login</source>
        <translatorcomment>Label to let the user start MEGAsync on login with a checkbox.</translatorcomment>
        <translation>Open at login</translation>
    </message>
    <message>
        <source>Show Mac OS notifications</source>
        <translatorcomment>Label to let the user enable Mac OS desktop notifications with a checkbox.</translatorcomment>
        <translation>Show Mac OS notifications</translation>
    </message>
    <message>
        <source>MEGAsync will perform a full scan of your synced folders when it starts.

Do you want to restart MEGAsync now?</source>
        <translatorcomment>Message displayed asking for confirmation to the user for a Full scan operation.</translatorcomment>
        <translation>MEGAsync will perform a full scan of your synced folders when it starts.Do you want to restart MEGAsync now?</translation>
    </message>
    <message>
        <source>Choose</source>
        <translatorcomment>Button label to choose a local/remote folder for downloads and uploads.</translatorcomment>
        <translation>選擇</translation>
    </message>
    <message>
        <source>Default folders</source>
        <translatorcomment>Label to indicate the default path for Uploads and Downloads.</translatorcomment>
        <translation>默認資料夾</translation>
    </message>
    <message>
        <source>Uploads:</source>
        <translatorcomment>Label to indicate the default folder for uploads. String as short as possible</translatorcomment>
        <translation>上傳：</translation>
    </message>
    <message>
        <source>Downloads:</source>
        <translatorcomment>Label to indicate the default folder for downloads. String as short as possible</translatorcomment>
        <translation>下載：</translation>
    </message>
    <message>
        <source>This sync can&apos;t be enabled because the local folder doesn&apos;t exist</source>
        <translatorcomment>Message displayed when an error occurs with the local folder.</translatorcomment>
        <translation>本地資料夾不存在，無法同步</translation>
    </message>
    <message>
        <source>This sync can&apos;t be enabled because the remote folder doesn&apos;t exist</source>
        <translatorcomment>Message displayed when an error occurs with the remote folder.</translatorcomment>
        <translation>This sync can&apos;t be enabled because the remote folder doesn&apos;t exist</translation>
    </message>
    <message>
        <source>You are already syncing your entire Cloud Drive</source>
        <translation type="obsolete">您已經同步您的整個雲端</translation>
    </message>
    <message>
        <source>Enable / disable</source>
        <translatorcomment>Tooltip to let the user enable/disable a specific synchronization.</translatorcomment>
        <translation>啟用 / 禁用</translation>
    </message>
    <message>
        <source>Select local folder</source>
        <translatorcomment>Title of the dialog to select the local folder for downloads.</translatorcomment>
        <translation>選擇本機資料夾</translation>
    </message>
    <message>
        <source>You don&apos;t have write permissions in this local folder.</source>
        <translatorcomment>Message displayed when a user is trying to download a file to a folder without write permissions.</translatorcomment>
        <translation>您沒有這個本地資料夾的編寫權限。</translation>
    </message>
    <message>
        <source>Export Key</source>
        <translatorcomment>Button label to export master key of the current user. String as short as possible.</translatorcomment>
        <translation type="unfinished">Export Key</translation>
    </message>
    <message>
        <source>Export Master key</source>
        <translatorcomment>Title of dialog to export master key for the current user.</translatorcomment>
        <translation type="unfinished">Export Master key</translation>
    </message>
    <message>
        <source>Unable to write file</source>
        <translatorcomment>Message displayed when an error occurs exporting the master key to a file.</translatorcomment>
        <translation type="unfinished">Unable to write file</translation>
    </message>
    <message>
        <source>Exporting the master key and keeping it in a secure location enables you to set a new password without data loss.</source>
        <translatorcomment>Label to inform the user to keep the master key in a secure location.</translatorcomment>
        <translation type="unfinished">正在導出您的萬能密鑰，請妥善保管。您的萬能密鑰使您不會因為密碼重置而丟失已上傳的數據。</translation>
    </message>
    <message>
        <source>Always keep physical control of your master key (e.g. on a client device, external storage, or print).</source>
        <translatorcomment>Label to inform the user to keep physical control of the master key.</translatorcomment>
        <translation type="unfinished">請確實掌握保存您的master key金鑰(如存於client端設備，外部儲存空間，或列印出)</translation>
    </message>
    <message>
        <source>Exclude by size</source>
        <translatorcomment>Button label to let the user exclude files from being synced by its size. String as short as possible.</translatorcomment>
        <translation type="unfinished">Exclude by size</translation>
    </message>
    <message>
        <source>PRO lite</source>
        <translatorcomment>Label to indicate that the current user has a PRO Iite account.</translatorcomment>
        <translation type="unfinished">PRO lite</translation>
    </message>
    <message>
        <source>The new excluded file names will be taken into account
when the application starts again</source>
        <translatorcomment>Message displayed to inform the user that the new file name exclusion changes will be applied on next startup.</translatorcomment>
        <translation type="unfinished">The new excluded file names will be taken into accountwhen the application starts again</translation>
    </message>
    <message>
        <source>The new excluded file sizes will be taken into account when the application starts again.</source>
        <translatorcomment>Message displayed to inform the user that the new file size exclusion changes will be applied on next startup.</translatorcomment>
        <translation type="unfinished">The new excluded file sizes will be taken into account when the application starts again.</translation>
    </message>
    <message>
        <source>Disabled</source>
        <translatorcomment>Label to indicate the user if the feature of exclusion based by size is enabled/disabled.</translatorcomment>
        <translation type="unfinished">Disabled</translation>
    </message>
    <message>
        <source>Local cache: %1</source>
        <translation type="unfinished">Local cache: %1</translation>
    </message>
    <message>
        <source>Remote cache: %1</source>
        <translation type="unfinished">Remote cache: %1</translation>
    </message>
</context>
<context>
    <name>SetupWizard</name>
    <message>
        <source>Setup Wizard - MEGAsync</source>
        <translatorcomment>Title of the dialog Setup Wizar of MEGAsync. Keep capital letters.</translatorcomment>
        <translation>Setup Wizard - MEGAsync</translation>
    </message>
    <message>
        <source>I have a MEGA account</source>
        <translatorcomment>Label to indicate the user has already a MEGA account with a checkbox.</translatorcomment>
        <translation>我有MEGA帳號</translation>
    </message>
    <message>
        <source>I don&apos;t have a MEGA account</source>
        <translatorcomment>Label to indicate if the user hasn&apos;t already a MEGA account with a checkbox.</translatorcomment>
        <translation>我沒有MEGA帳號</translation>
    </message>
    <message>
        <source>Create a new MEGA account</source>
        <translatorcomment>Label displayed when a user is creating a new MEGA account.</translatorcomment>
        <translation>建立新的MEGA帳號</translation>
    </message>
    <message>
        <source>Name:</source>
        <translatorcomment>Label for field name at create account. String as short as possible.</translatorcomment>
        <translation>名稱:</translation>
    </message>
    <message>
        <source>Email:</source>
        <translatorcomment>Label for field email at create account. String as short as possible.</translatorcomment>
        <translation>電子郵件:</translation>
    </message>
    <message>
        <source>Password:</source>
        <translatorcomment>Label for field password at create account. String as short as possible.</translatorcomment>
        <translation>密碼:</translation>
    </message>
    <message>
        <source>Repeat password:</source>
        <translatorcomment>Label for field repeat password at create account. String as short as possible.</translatorcomment>
        <translation>重複一次密碼:</translation>
    </message>
    <message>
        <source>I agree with the MEGA &lt;a href=&quot;https://mega.co.nz/#terms&quot;&gt;Terms of Service&lt;/a&gt;</source>
        <translatorcomment>Label to aggre with the Terms of use.  Keep code &lt;a href=&quot;https://mega.co.nz/#terms&quot;&gt;Terms of Service&lt;/a&gt; .String as short as possible.</translatorcomment>
        <translation>I agree with the MEGA <a href=&quot;https://mega.co.nz/#terms&quot;>Terms of Service</a></translation>
    </message>
    <message>
        <source>Login to your MEGA account</source>
        <translatorcomment>Label to let the user login with his credentials.</translatorcomment>
        <translation>登入您的MEGA帳號</translation>
    </message>
    <message>
        <source>Please verify your account using the confirmation link that we have sent to your email account</source>
        <translatorcomment>Label displayed at last step of creation account process to inform the user to verify the new created account.</translatorcomment>
        <translation>請透過我們寄到您電子郵件信箱的確認連結來認證您的帳號</translation>
    </message>
    <message>
        <source>Logging in ...</source>
        <translatorcomment>Label displayed while logging process.</translatorcomment>
        <translation>Logging in ...</translation>
    </message>
    <message>
        <source>Choose install type</source>
        <translatorcomment>Label displayed to inform the user about the installation type for the synchronizations (selective or full sync)</translatorcomment>
        <translation>選擇安裝類型</translation>
    </message>
    <message>
        <source> Sync your entire cloud drive</source>
        <translatorcomment>Label displayed to inform the user about the Full sync (Sync the entire cloud drive)</translatorcomment>
        <translation>同步您的整個雲端硬碟</translation>
    </message>
    <message>
        <source>Full account sync</source>
        <translation type="obsolete">全帳號同步</translation>
    </message>
    <message>
        <source>Selective sync</source>
        <translatorcomment>Label displayed to show Selective sync mode. Max 18 characters.</translatorcomment>
        <translation>部分同步</translation>
    </message>
    <message>
        <source> Sync specific folders in your cloud drive</source>
        <translatorcomment>Label displayed to inform the user about the Selective  sync (Sync specific folders)</translatorcomment>
        <translation>同步選定的雲端硬碟資料夾</translation>
    </message>
    <message>
        <source>The following folders will be automatically synchronized:</source>
        <translatorcomment>Label to inform the user about the folders wich will be synchronized.</translatorcomment>
        <translation>下列資料夾將自動同步:</translation>
    </message>
    <message>
        <source>Local folder:</source>
        <translatorcomment>Label displayed to show the local folder synchronized.</translatorcomment>
        <translation>本機資料夾:</translation>
    </message>
    <message>
        <source>MEGA folder:</source>
        <translatorcomment>Label displayed to show the MEGA folder synchronized.</translatorcomment>
        <translation>MEGA資料夾:</translation>
    </message>
    <message>
        <source>Change</source>
        <translatorcomment>Button label to change the local or remote folder during the process of createn a new synchronization. String as short as possible.</translatorcomment>
        <translation>變更</translation>
    </message>
    <message>
        <source>Welcome to MEGA</source>
        <translatorcomment>Label displayed to welcome the user.</translatorcomment>
        <translation>歡迎使用MEGA</translation>
    </message>
    <message>
        <source>Your local folder:</source>
        <translation type="obsolete">您的本地資料夾:</translation>
    </message>
    <message>
        <source>and your MEGA folder:</source>
        <translation type="obsolete">與您的MEGA資料夾:</translation>
    </message>
    <message>
        <source>will be automatically synchronized.</source>
        <translation type="obsolete">將自動同步。</translation>
    </message>
    <message>
        <source>Back</source>
        <translatorcomment>Button label to let the user go back through the wizard assistant.</translatorcomment>
        <translation>返回</translation>
    </message>
    <message>
        <source>Next</source>
        <translatorcomment>Button label to let the user go next through the wizard assistant.</translatorcomment>
        <translation>次一</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Button label to let the user cancel the wizard assistant.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Error</source>
        <translatorcomment>Label of error.</translatorcomment>
        <translation>錯誤</translation>
    </message>
    <message>
        <source>User already exists</source>
        <translatorcomment>Message displayed when a user is trying to create an account with the name of an existing user.</translatorcomment>
        <translation>使用者已存在</translation>
    </message>
    <message>
        <source>Fetching file list...</source>
        <translatorcomment>Label displayed while the application is retrieving all file list from the server.</translatorcomment>
        <translation>Fetching file list...</translation>
    </message>
    <message>
        <source>Incorrect email and/or password.</source>
        <translatorcomment>Message displayed when a user is trying to loging with an incorrect email/password.</translatorcomment>
        <translation>不正確的電子郵件與/或密碼。</translation>
    </message>
    <message>
        <source>Have you verified your account?</source>
        <translatorcomment>Label displayed to inform the user about the need of verify the created account.</translatorcomment>
        <translation>您認證過帳號了嗎?</translation>
    </message>
    <message>
        <source>MEGA folder doesn&apos;t exist</source>
        <translatorcomment>Label displayed when an error occurs with a remote folder.</translatorcomment>
        <translation>MEGA資料夾不存在</translation>
    </message>
    <message>
        <source>Finish</source>
        <translatorcomment>Button label to let the user that the wizard assistant has finished.</translatorcomment>
        <translation>完成</translation>
    </message>
    <message>
        <source>Please, enter your e-mail address</source>
        <translatorcomment>Label displayed when the user has not filled the email field.</translatorcomment>
        <translation>Please, enter your e-mail address</translation>
    </message>
    <message>
        <source>Please, enter a valid e-mail address</source>
        <translatorcomment>Label displayed when the user has filled an invalid email address.</translatorcomment>
        <translation>Please, enter a valid e-mail address</translation>
    </message>
    <message>
        <source>Please, enter your password</source>
        <translatorcomment>Label displayed when the user has not filled the password field.</translatorcomment>
        <translation>Please, enter your password</translation>
    </message>
    <message>
        <source>Please, enter your name</source>
        <translatorcomment>Label displayed when the user has not filled the name field.</translatorcomment>
        <translation>Please, enter your name</translation>
    </message>
    <message>
        <source>Please, enter a stronger password</source>
        <translatorcomment>Label displayed when the user has filled a password not enough secure.</translatorcomment>
        <translation>Please, enter a stronger password</translation>
    </message>
    <message>
        <source>The entered passwords don&apos;t match</source>
        <translatorcomment>Label displayed when the user has filled different password.</translatorcomment>
        <translation>輸入的密碼不符</translation>
    </message>
    <message>
        <source>You have to accept our terms of service</source>
        <translatorcomment>Label displayed when the user has not check the agreement of terms of service.</translatorcomment>
        <translation>您必須同意我們的服務條款</translation>
    </message>
    <message>
        <source>Creating account...</source>
        <translatorcomment>Label displayed when the account is being created.</translatorcomment>
        <translation>Creating account...</translation>
    </message>
    <message>
        <source>Warning</source>
        <translatorcomment>Label warning.</translatorcomment>
        <translation>警告</translation>
    </message>
    <message>
        <source>You have %1 in your Cloud Drive.
Are you sure you want to sync your entire Cloud Drive?</source>
        <translation type="obsolete">You have %1 in your Cloud Drive.Are you sure you want to sync your entire Cloud Drive?</translation>
    </message>
    <message>
        <source>and your MEGA Cloud Drive</source>
        <translation type="obsolete">與您的MEGA雲端硬碟</translation>
    </message>
    <message>
        <source>Please, select a local folder</source>
        <translatorcomment>Label displayed when the user has not select a local folder.</translatorcomment>
        <translation>Please, select a local folder</translation>
    </message>
    <message>
        <source>Please, select a MEGA folder</source>
        <translatorcomment>Label displayed when the user has not select a MEGA folder.</translatorcomment>
        <translation>Please, select a MEGA folder</translation>
    </message>
    <message>
        <source>Local folder too large (this version is limited to %1 folders or %2 files.
Please, select another folder.</source>
        <translation type="obsolete">Local folder too large (this version is limited to %1 folders or %2 files.Please, select another folder.</translation>
    </message>
    <message>
        <source>Select local folder</source>
        <translatorcomment>Label displayed to let the user select a local folder for a synchronization.</translatorcomment>
        <translation>選擇本機資料夾</translation>
    </message>
    <message>
        <source>Logging in...</source>
        <translatorcomment>Label displayed when the user is logging in.</translatorcomment>
        <translation>Logging in...</translation>
    </message>
    <message>
        <source>You are trying to sync an extremely large folder.
To prevent the syncing of entire boot volumes, which is inefficient and dangerous,
we ask you to start with a smaller folder and add more data while MEGAsync is running.</source>
        <translatorcomment>Message displayed to advise the user that is trying to sync an extremely large folder.</translatorcomment>
        <translation>You are trying to sync an extremely large folder.To prevent the syncing of entire boot volumes, which is inefficient and dangerous,we ask you to start with a smaller folder and add more data while MEGAsync is running.</translation>
    </message>
    <message>
        <source>Unable to get the filesystem.
Please, try again. If the problem persists please contact bug@mega.co.nz</source>
        <translatorcomment>Message displayed when a problem occurs while fetching filesystem from the cloud drive.</translatorcomment>
        <translation>Unable to get the filesystem.Please, try again. If the problem persists please contact bug@mega.co.nz</translation>
    </message>
    <message>
        <source>Setup Assistant - MEGAsync</source>
        <translatorcomment>Title of the setup assistant dialog. Keep capital letters.</translatorcomment>
        <translation>Setup Assistant - MEGAsync</translation>
    </message>
    <message>
        <source>Error getting session key</source>
        <translatorcomment>Message displayed when an error occurs checking session key</translatorcomment>
        <translation>取得session key時發生錯誤</translation>
    </message>
    <message>
        <source>Full sync</source>
        <translatorcomment>Label displayed to show Full sync mode. Max 18 characters.</translatorcomment>
        <translation>完全同步</translation>
    </message>
    <message>
        <source>Your local folder and your MEGA Cloud Drive will be automatically synchronized.</source>
        <translatorcomment>Label to inform the user that the synchronization stablished will be automatically synchronized.</translatorcomment>
        <translation>您的本機資料夾與您的MEGA雲端硬碟將自動同步。</translation>
    </message>
    <message>
        <source>Your Cloud Drive will be synchronized with this folder:</source>
        <translatorcomment>Label to inform the user wich local folder will be synchronized with the cloud drive. Keep colon.</translatorcomment>
        <translation>您的云端將與這個資料夾同步</translation>
    </message>
    <message>
        <source>You don&apos;t have write permissions in this local folder.</source>
        <translatorcomment>Message displayed when a user is trying to synchronized a local folder in wich the user has no write permissions.</translatorcomment>
        <translation>您沒有這個本地資料夾的編寫權限。</translation>
    </message>
    <message>
        <source>MEGAsync won&apos;t be able to download anything here.</source>
        <translatorcomment>Message displayed when a user is trying to synchronized a local folder in wich the user has no write permissions.</translatorcomment>
        <translation>MEGAsync 無法從這裡下載任何資料</translation>
    </message>
    <message>
        <source>Do you want to continue?</source>
        <translatorcomment>Message of confirmation to continue with the current operation.</translatorcomment>
        <translation>您要繼續嗎？</translation>
    </message>
    <message>
        <source>Your account has been blocked. Please contact support@mega.co.nz</source>
        <translatorcomment>Message displayed when an account has been blocked.</translatorcomment>
        <translation type="unfinished">Your account has been blocked. Please contact support@mega.co.nz</translation>
    </message>
    <message>
        <source>MEGAsync</source>
        <translation type="unfinished">MEGAsync</translation>
    </message>
    <message>
        <source>Are you sure you want to cancel this wizard and undo all changes?</source>
        <translation type="unfinished">Are you sure you want to cancel this wizard and undo all changes?</translation>
    </message>
</context>
<context>
    <name>ShellExtension</name>
    <message>
        <source>Upload to MEGA</source>
        <translatorcomment>Label displayed when a user is trying to upload a file/folder to MEGA from the shell extension. String as short as possible.</translatorcomment>
        <translation>上傳至MEGA</translation>
    </message>
    <message>
        <source>Get MEGA link</source>
        <translatorcomment>Label displayed when a user is trying to get public link of a file/folder to MEGA from the shell extension. String as short as possible.</translatorcomment>
        <translation>取得MEGA連結</translation>
    </message>
    <message>
        <source>Share with a MEGA user</source>
        <translatorcomment>Label displayed when a user is trying to share a public link of a file/folder with a user. String as short as possible.</translatorcomment>
        <translation>與MEGA使用者分享</translation>
    </message>
    <message>
        <source>Send to a MEGA user</source>
        <translatorcomment>Label displayed when a user is trying to send a public link of a file/folder to a user. String as short as possible.</translatorcomment>
        <translation>寄給MEGA使用者</translation>
    </message>
    <message>
        <source>1 file</source>
        <translatorcomment>Label to indicate one file.</translatorcomment>
        <translation>一個檔案</translation>
    </message>
    <message>
        <source>%1 files</source>
        <translatorcomment>Label to indicate several file. Keep %1 code because it will be filled with the number of files at runtime.</translatorcomment>
        <translation>%1 個檔案</translation>
    </message>
    <message>
        <source>1 folder</source>
        <translation>一個資料夾</translation>
    </message>
    <message>
        <source>%1 folders</source>
        <translatorcomment>Label to indicate several folders. Keep %1 code because it will be filled with the number of folders at runtime.</translatorcomment>
        <translation>%1 個資料夾</translation>
    </message>
    <message>
        <source>%1 (%2, %3)</source>
        <translatorcomment>Keep %1 (%2, %3) code because it will be filled with the number of folders at runtime.</translatorcomment>
        <translation>%1 (%2, %3)</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translatorcomment>Keep %1 (%2) code because it will be filled with the number of folders at runtime.</translatorcomment>
        <translation>%1 (%2)</translation>
    </message>
</context>
<context>
    <name>SizeLimitDialog</name>
    <message>
        <source>Exclude by size</source>
        <translatorcomment>Title of the dialog to set exclusion based on file size.</translatorcomment>
        <translation type="unfinished">Exclude by size</translation>
    </message>
    <message>
        <source>Exclude files bigger than</source>
        <translatorcomment>Label to indicate the upper limit for file exclusions. Max 28 characters.</translatorcomment>
        <translation type="unfinished">Exclude files bigger than</translation>
    </message>
    <message>
        <source>Exclude files smaller than</source>
        <translatorcomment>Label to indicate the lower limit for file exclusions. Max 28 characters.</translatorcomment>
        <translation type="unfinished">Exclude files smaller than</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation type="unfinished">取消</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>Warning</source>
        <translatorcomment>Label warning.</translatorcomment>
        <translation type="unfinished">警告</translation>
    </message>
    <message>
        <source>Size limits cannot be zero</source>
        <translatorcomment>Message displayed when a user is trying to set zero limits.</translatorcomment>
        <translation type="unfinished">Size limits cannot be zero</translation>
    </message>
</context>
<context>
    <name>UploadToMegaDialog</name>
    <message>
        <source>Upload to MEGA</source>
        <translatorcomment>Label and Title of the dialog displayed when a user is trying to upload a file/folder to MEGA.(MAX 20 characters)</translatorcomment>
        <translation>上傳至MEGA</translation>
    </message>
    <message>
        <source>Please, select the upload folder for your files:</source>
        <translatorcomment>Label to inform the user of the destination local folder for the files to be uploaded (MAX 50 characters)</translatorcomment>
        <translation>Please, select the upload folder for your files:</translation>
    </message>
    <message>
        <source>MEGA folder:</source>
        <translatorcomment>Label to indicate the user the MEGA folder in which the selected files/folders will be uploaded (String short as possible)</translatorcomment>
        <translation>MEGA資料夾:</translation>
    </message>
    <message>
        <source>Always upload to this destination</source>
        <translatorcomment>Label to let the user select a default upload folder with a checkbox.</translatorcomment>
        <translation>每次上傳都以此為目的地資料夾</translation>
    </message>
    <message>
        <source>/MEGAsync Uploads</source>
        <translatorcomment>Label to indicate the default MEGA folder for file uploaded. Keep / symbol.</translatorcomment>
        <translation>/MEGAsync Uploads</translation>
    </message>
    <message>
        <source>MEGAsync Uploads</source>
        <translatorcomment>Label to indicate the default MEGA folder for file uploaded.</translatorcomment>
        <translation>MEGAsync Uploads</translation>
    </message>
    <message>
        <source>OK</source>
        <translatorcomment>Label for accept button.</translatorcomment>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translatorcomment>Label for cancel button.</translatorcomment>
        <translation>取消</translation>
    </message>
    <message>
        <source>Choose</source>
        <translatorcomment>Button label to select the upload folder (String as short as possible)</translatorcomment>
        <translation>選擇</translation>
    </message>
</context>
<context>
    <name>UsageProgressBar</name>
    <message>
        <source>Inbox</source>
        <translation type="unfinished">收件夾</translation>
    </message>
    <message>
        <source>Incoming Shares</source>
        <translation type="unfinished">傳入的分享</translation>
    </message>
    <message>
        <source> Rubbish Bin</source>
        <translation type="unfinished">垃圾夾</translation>
    </message>
    <message>
        <source>Cloud Drive</source>
        <translation type="unfinished">雲端硬碟</translation>
    </message>
</context>
<context>
    <name>UsageWidget</name>
    <message>
        <source>Cloud Drive</source>
        <translation type="unfinished">雲端硬碟</translation>
    </message>
    <message>
        <source>Rubbish Bin</source>
        <translation type="unfinished">垃圾夾</translation>
    </message>
    <message>
        <source>Incoming Shares</source>
        <translation type="unfinished">傳入的分享</translation>
    </message>
    <message>
        <source>Inbox</source>
        <translation type="unfinished">收件夾</translation>
    </message>
    <message>
        <source>Used</source>
        <translation type="unfinished">已用</translation>
    </message>
    <message>
        <source>Available</source>
        <translation type="unfinished">可用</translation>
    </message>
</context>
<context>
    <name>WindowsPlatform</name>
    <message>
        <source>MEGA synced folder</source>
        <translatorcomment>Label to indicate the synced MEGA folder.</translatorcomment>
        <translation>MEGA同步資料夾</translation>
    </message>
</context>
</TS>

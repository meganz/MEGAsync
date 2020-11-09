echo "" > .\fileswin_new.txt
cd MEGAsync
for /R %%f in (.\*) do powershell -command "get-filehash -algorithm SHA256 '%%f'  | Format-Table -AutoSize | Out-File -append -encoding ascii ..\fileswin_new.txt"

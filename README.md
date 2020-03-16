# FolderTransfer-Windows
Copies an folder over one TCP connection


Server (Receiver)             Client (Sender)

FileHeader                    FileHeadear
<FileName>,<FileSize>         <FileName>,<FileSize>
<FileData>                    <FileData>
RECV_OK                       RECV_OK

... (loop)                    ... loop

[[FOLDER_SEND_END]]           [[FOLDER_SEND_END]]

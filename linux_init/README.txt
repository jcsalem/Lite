To install this startup script on Debian server:
 sudo cp <script> /etc/init.d
 sudo chmod +x <script> /etc/init.d/<script>
 sudo update-rc.d <script> defaults
Where <script> is literun or liteberryrun

To Test
 /etc/init.d literun start
OR
 sudo liteberryrun start
OR 
 same but stop or restart

To remove
 sudo update-rc.d <sudo> remote

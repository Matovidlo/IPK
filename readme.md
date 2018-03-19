# NAME
**ftrestd** \- server which communicates within HTTP protocol
# SYNOPSIS
**ftrestd [ -r ROOT-FOLDER ] [-p PORT ]**
#DESCRIPTION
**ftrestd**
is a simple server, which supports RESTFul APIs and provides simple communication. Server can upload (GET) file, download (PUT) file, create or remove directory, list directory contents. Remote upload or download is performed by client ftrest.
#OPTIONS
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*-r ROOT-FOLDER*
Specifies root folder where are contents stored, downloaded.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*-p PORT*
Port is a number, on which server is listening to incomming remote communication.
#IMPLEMENTATION
The server is implemented as IPv6 concurrent server. It is listening on port for any inbound communication. After listening phase server accepts communication. At first, server waits for BSD socket from client which is supposed to be REST header, server checks wether it is REST header. Within loop is accepting packets where should be body of REST message. According to REST command the server execute it - if any error occurs, message contains information about failure on server side. The server ends when SIGINT signal is given (Ctrl-C).
#ERRORS
In case of wrong arguments or network problems(connect,accept, etc..), a program exits with code 1. If something goes wrong when performing operations with file or directory, an error message is sent back to the remote.
#EXAMPLES
**ftrestd -p 8888**
: initiate a connection with port 8888 and a root directory is default(current directory).

**ftrestd -r /home/test/folder**
: initiate a connection with default port 6677 and a root directory is set as `/home/test/folder`.

**ftrestd -r /home/test -p 8888**
: initiate a connection with port 8888 and a root directory is set as `/home/test/folder`.

#AUTHOR
Martin Vaško <xvasko12@fit.vutbr.cz>
#SEE ALSO
**ftrest** \- a REST client
#NAME
**ftrest** \- a REST client which communicates with a ftrestd server using RESTful APIs.
#SYNOPSIS
**ftrest COMMAND REMOTE-PATH [ LOCAL-PATH ]**
#OPTIONS
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*COMMAND*
Should be a RESTful operation as del, get, put, lst, mkd, rmd.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*REMOTE-PATH*
Path on remote side (ftrestd server).

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*LOCAL-PATH*
Path in Linux filesystem where should be file copied from (put) or stored to (get).
#DESCRIPTION
**ftrest**
is a simple client which uses RESTful APIs to communicate with a ftrestd server. A user can use this client to upload, download or remove file. Also client supports work with folder. Create or remove (empty) directory or list contents of directory. All of those operations are performed on server side performed.
#IMPLEMENTATION
The client is implemented as IPv4 client. Processes command line options and parses them into smaller pieces which contains - serverHostname (also IP ADDRESS), portNumber and realPath where is specified where on server should be action performed. From those pieces is created HTTP header which is sent to server. Client waits for reply message from server which contains HTTP status code which indicates, wether server was able to perform action or request was specified wrong. Code 200 means that action was performed. Commands as `get` and `lst` also have data which are sent on standard output.
#COMMAND
**put**
:   Copy file defined by *LOCAL-PATH* to directory defined by *REMOTE-PATH* on the server.

**del**
:   Delete file defined by *REMOTE-PATH* on the server.

**get**
:   Copy file defined by *REMOTE-PATH* on the server to current local directory or at the place defined by *LOCAL-PATH*, if set.

**lst**
:   Print content of directory defined by *REMOTE-PATH* to standard output stream (same format as *ls*).

**mkd**
:   Create directory defined by *REMOTE-PATH* on the server.

**rmd**
:   Delete directory defined by *REMOTE-PATH* on the server (directory must be empty).
#REMOTE PATH
   **[ protocol :// ]hostname:port/path**

   if protocol is not defined, http is used as a default protocol.

#Example
   **http://localhost:12345/tonda/foo/bar**

   protocol is http, hostname is localhost, port is 12345 and path is tonda/foo/bar.
#ERRORS
**Not a directory.**
:   Occurs when *REMOTE-PATH* is a file when processing operation lst, mkd or rmd.

**Directory not found.**
:   Occurs when directory defined *REMOTE-PATH* does not exist when processing operation lst, mkd or rmd.

**Directory not empty.**
:   Occurs when directory defined *REMOTE-PATH* is not empty when processing operation rmd.

**Already exists.**
:   Occurs when directory  or file defined *REMOTE-PATH* already exists when processing operation mkd or put.

**Not a file.**
:   Occurs when *REMOTE-PATH* is a directory when processing operation put or get.

**File not found.**
:   Occurs when file defined by*REMOTE-PATH* does not exist when processing operation put or get.

**User Account Not Found**
:   Occurs when operation works with user directory which does not exists.

**Unknown error.**
:   For other errors when creating/removing/copying/moving operations failed on the server side.

#EXAMPLES
**ftrest mkd http://localhost:12345/tonda/foo/bar**
:   create the directory *foo* in the directory *bar* which is the subdirectory of the root directory of the user *tonda*.

**ftrest put http://localhost:12345/tonda/foo/bar/doc.pdf ~/doc.pdf**
:   upload the local file *doc.pdf* into the directory *bar* on the server.

**ftrest get http://localhost:12345/tonda/foo/bar/doc.pdf**
:   download the remote file *doc.pdf* into the current local directory.

**ftrest del http://localhost:12345/tonda/foo/bar/doc.pdf**
:   delete the remote file *doc.pdf*.

**ftrest rmd http://localhost:12345/tonda/foo/bar**
:   delete the directory *bar* on the server.

**ftrest lst http://localhost:12345/tonda**
:   print content of the directory *tonda*.

#AUTHOR
Martin Vaško <xvasko12@fit.vutbr.cz>
#SEE ALSO
**ftrestd** \- a REST server.

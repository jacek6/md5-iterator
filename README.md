# md5-iterator
Bacis cpp iterator over md5 hashes
You may run it by:
```
$ build/proj2 dict.txt md5.txt
argc = 3
Loaded 4 passwords
zalamano haslo 11Air78
zlamane haslo '11Air78' dla 7FF0A779EE994A1760675A569555D02E 11Air78
```

First arg is file with a dict, second arg is a file with MD5 hashes.

Dict file, contains word in each line.

MD5 hashes file contains in each line hashes:
 - each line start with MD5 hash
 - rest of the line, after MD5 hash is any other information (for example mail of given hash), this information will be shown after cracking MD5 hash 

Example content ro MD5 hash file:
```
31D9BB37999652D494BA78FEB642A73F jakis_mail@gmail.com
ceD4A8463B37BF11F394C31DB455EFC1 inny.mail@gmail.com
C594CE5169860AECFBD4A1DB001467F7 trudniejszy...
7FF0A779EE994A1760675A569555D02E 11Air78
```

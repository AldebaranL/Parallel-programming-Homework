1.����
2.����cmd
cd /d D:\Mycodes\VSProject\ANN_final\Debug
start /b smpd -d
mpiexec -hosts 1 localhost 4 .\ANN_final.exe
����
mpiexec -n .\hello.exe

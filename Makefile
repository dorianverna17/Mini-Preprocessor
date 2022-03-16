+CC=cl.exe
+LINK=link.exe

build :
	$(LINK) /OUT:so-cpp so-cpp.obj
so-cpp.obj : so-cpp.c
	$(CC) /c so-cpp.c /MD so-cpp.obj
clean :
	del so-cpp

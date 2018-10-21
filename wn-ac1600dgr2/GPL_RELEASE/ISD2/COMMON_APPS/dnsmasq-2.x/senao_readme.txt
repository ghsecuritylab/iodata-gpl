20130927 Jason: Turn On printf

diff --git a/COMMON_APPS/dnsmasq-2.x/src/dnsmasq.c b/COMMON_APPS/dnsmasq-2.x/src/dnsmasq.c
index 737fb1a..bb08d08 100644
--- a/COMMON_APPS/dnsmasq-2.x/src/dnsmasq.c
+++ b/COMMON_APPS/dnsmasq-2.x/src/dnsmasq.c
@@ -127,9 +127,9 @@ int main (int argc, char **argv)
 #endif
 #endif  
   /* Close any file descriptors we inherited apart from std{in|out|err} */
-  for (i = 0; i < max_fd; i++)
-    if (i != STDOUT_FILENO && i != STDERR_FILENO && i != STDIN_FILENO)
-      close(i);
+//  for (i = 0; i < max_fd; i++)
+//    if (i != STDOUT_FILENO && i != STDERR_FILENO && i != STDIN_FILENO)
+//      close(i);
 
 #ifdef HAVE_LINUX_NETWORK
   netlink_init();
@@ -352,11 +352,11 @@ int main (int argc, char **argv)
        }
          
       /* open  stdout etc to /dev/null */
-      nullfd = open("/dev/null", O_RDWR);
-      dup2(nullfd, STDOUT_FILENO);
-      dup2(nullfd, STDERR_FILENO);
-      dup2(nullfd, STDIN_FILENO);
-      close(nullfd);
+      //nullfd = open("/dev/null", O_RDWR);
+      //dup2(nullfd, STDOUT_FILENO);
+      //dup2(nullfd, STDERR_FILENO);
+      //dup2(nullfd, STDIN_FILENO);
+      //close(nullfd);
     }


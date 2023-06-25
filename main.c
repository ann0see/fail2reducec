/*
 * Copyright <2023> <ann0see>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// A program to buffer strings and later on send them in bulk via E-Mail in a
// file. Developed to get rid of multiple fail2ban startup messages
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// append mail text to buffer file
int appendMail(const char *mailText, const char *bufferFilePath) {

  FILE *bufferFileHandler = fopen(bufferFilePath, "a");
  if (bufferFileHandler == NULL) {
    // couldn't open buffer file
    return -1;
  }

  // secure appending to file with lock and later on write to buffer
  if (flock(fileno(bufferFileHandler), LOCK_EX) == -1) {
    fprintf(stderr, "Couldn't lock buffer file.\n");
    return -1;
  }

  fprintf(bufferFileHandler, "%s\n", mailText);

  flock(fileno(bufferFileHandler), LOCK_UN);

  return fclose(bufferFileHandler);
}

int main(int argc, char *argv[]) {
  if (argc != 4) { // wrong argument count --> display help
    printf("Usage: <text> <buffer_file_path> <lock_file_path>\n");
    return 1;
  }

  const char *mailText = argv[1];
  const char *bufferFilePath = argv[2];
  const char *lockfile = argv[3];

  // Create lockfile if needed and check if we are the first one running.
  int lockfd = open(lockfile, O_CREAT, 0600);
  if (lockfd == -1) {
    fprintf(stderr, "Couldn't open lock file\n");
    return 1;
  }

  int lockFileStatus = flock(lockfd, LOCK_EX | LOCK_NB);
  if (lockFileStatus == -1) {
    // File not lockable, thus we assume another process is running. This means
    // that we do not need to handle checking and email sending
    return appendMail(mailText, bufferFilePath);
  } else {
    // As the file is lockable, we need to perform the waiting/buffering process. Output that this is the case for the bash script
    printf("monitors_file");
    int appendMailStatus = appendMail(mailText, bufferFilePath);
    if (appendMailStatus != 0) {
      fprintf(stderr, "Couldn't append to buffer file.\n");
      return appendMailStatus;
    }

    while (true) {
      // Get time of modification of the bufferFile
      struct stat statattr;
      stat(bufferFilePath, &statattr); // get the stats like last modification
                                       // time of the buffer file
      time_t lastModified = statattr.st_mtime;
      time_t currentTime = time(NULL);
      time_t timeDifference = currentTime - lastModified;

      if (timeDifference > 10) {
        // file has not been modified for 11 seconds, thus we assume that we can
        // abort --> send file content to stdout and delete file
        FILE *bufferFileHandler = fopen(bufferFilePath, "r");
        if (bufferFileHandler == NULL) {
          fprintf(stderr, "Couldn't open and read buffer file.\n");
          return -1;
        }
        // further handelling like mailing out is done via a bash script. We'd
        // also use flock there
        break;
      } else {
        sleep(11 - timeDifference); // sleep approximately the needed time
      }
    }

    // unlocking but not deleting. Presence of lockfile doesn't mean the file is
    // locked in Linux
    flock(lockfd, LOCK_UN);
    close(lockfd);
  }
}

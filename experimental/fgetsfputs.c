#include <stdio.h>

int main() {
    FILE *sourceFile, *destinationFile;
    char buffer[600];

    // Open the source file in read mode
    sourceFile = fopen("sample.txt", "rb");
    if (sourceFile == NULL) {
        printf("Error opening source file.\n");
        return 1;
    }

    // Open the destination file in write mode
    destinationFile = fopen("dest.txt", "wb");
    if (destinationFile == NULL) {
        printf("Error opening destination file.\n");
        fclose(sourceFile);
        return 1;
    }

    // Read from the source file and write to the destination file until end of file
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
        printf("\n--------------------------------------------------------\n%s\n------------------------------------------------------\n", buffer);
        fwrite(buffer, 1, bytesRead, destinationFile);
        fflush(destinationFile);
    }

    // Close the files
    fclose(sourceFile);
    fclose(destinationFile);

    printf("File transfer successful.\n");

    return 0;
}

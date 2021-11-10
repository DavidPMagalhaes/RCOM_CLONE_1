char headerProtection(char * toProtect){
    char toReturn = 0x00;
    char sum = 0x00;
    for(int i = 0; i < 3; i++){
        sum = sum ^ toProtect[i];
    }
    return sum;
}
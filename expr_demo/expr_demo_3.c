/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
  
//https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
// Function to replace a string with another 
// string 
char* replaceWord(const char* s, const char* oldW, 
                  const char* newW) 
{ 
    char* result; 
    int i, cnt = 0; 
    int newWlen = strlen(newW); 
    int oldWlen = strlen(oldW); 
  
    // Counting the number of times old word 
    // occur in the string 
    for (i = 0; s[i] != '\0'; i++) { 
        if (strstr(&s[i], oldW) == &s[i]) { 
            cnt++; 
  
            // Jumping to index after the old word. 
            i += oldWlen - 1; 
        } 
    } 
  
    // Making new string of enough length 
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1); 
  
    i = 0; 
    while (*s) { 
        // compare the substring with the result 
        if (strstr(s, oldW) == s) { 
            strcpy(&result[i], newW); 
            i += newWlen; 
            s += oldWlen; 
        } 
        else
            result[i++] = *s++; 
    } 
  
    result[i] = '\0'; 
    return result; 
} 

//https://stackoverflow.com/questions/9329406/evaluating-arithmetic-expressions-from-string-in-c
char peek(char** e)
{
    return **e;
}

char get(char** e)
{
    char ret = **e;
    ++*e;
    return ret;
}

int expression();

int number(char** e)
{
    int result = get(e) - '0';
    while (peek(e) >= '0' && peek(e) <= '9')
    {
        result = 10*result + get(e) - '0';
    }
    return result;
}

int factor(char** e)
{
    if (peek(e) >= '0' && peek(e) <= '9')
        return number(e);
    else if (peek(e) == '(')
    {
        get(e); // '('
        int result = expression(e);
        get(e); // ')'
        return result;
    }
    else if (peek(e) == '-')
    {
        get(e);
        return -factor(e);
    }
    return 0; // error
}

int term(char ** e)
{
    int result = factor(e);
    while (peek(e) == '*' || peek(e) == '/')
        if (get(e) == '*')
            result *= factor(e);
        else
            result /= factor(e);
    return result;
}

int expression_inner(char ** e)
{
    int result = term(e);
    while (peek(e) == '+' || peek(e) == '-')
        if (get(e) == '+')
            result += term(e);
        else
            result -= term(e);
    return result;
}

int expression(char * fos){
    return expression_inner(&fos);
}

int alma(char** a){
    ++(*a);
}

void insertTo(char* found,int foundLen,char* that){
    char ending[100] = {0};
    
    sprintf(ending,"%s",found+foundLen);
    sprintf(found,"%s",that);
    sprintf(found+strlen(that),"%s",ending);
}

void subst(char* str,char* vName,int v){
    int nameLen = strlen(vName);
    
    char* found = strstr(str,vName);
    int isVar = found[nameLen] != '(' ? 1 : 0;
    if(isVar == 0){
        nameLen = 0; //calc closing of found[nameLen] == '('
        // nameLen értéke legyen a pl abs(2*(4-1)*6) nal 14
    }
    
    
    // START: SUKU
    
    int nyitCount = 0;
    int zarCount = 0;
    
    for(int i=0; i<nameLen; i++){
        
        if (found[i] == '('){
            
            nyitCount++;
            printf("(\n");
        }
        else if (found[i] == ')'){
            zarCount++;
            
            printf(")\n");
            
            if (zarCount == nyitCount){
                
                printf("Hossz: %d\n", i+1);
                nameLen = i+1;
                
                char buff[100] = {0};
    
                sprintf(buff,"%d",v);
    
                insertTo(found,nameLen,buff);
                return;
                break;
            }
        }
        
    }
    
    // END: SUKU
    
    char buff[100] = {0};
    
    sprintf(buff,"%d",v);
    if(isVar==1){
        
        
    //itt csak simán jöhet az insertTo azt csá    
        insertTo(found,nameLen,buff);
    }else{
        //no good yet
        int l = strlen(buff);
        char tmp = found[l];
        sprintf(found,buff); //please no 0
        found[l] = tmp;
    }
}

int calcBuiltin(char* name,int value){
    
    if(strcmp(name,"abs")==0){
        return abs(value);
    }else if(strcmp(name,"six")){
        return 666666;
    }
}


void calcSubFnc(char* str, int funcpos){
    char* fName = str + funcpos;
    int fNameEnd = strstr(fName,"(") - fName;
    
    int innerexpr = expression(fName+fNameEnd+1); //genyo vagyok mer ugy veszem h csak a bezaroig parsol
    
    int resultOfFnc = calcBuiltin(fName,innerexpr);
    
    subst(str,fName,resultOfFnc);
}

int main()
{
    {
        char exprArr[100] = {0};
        sprintf(exprArr,"1+10");
        printf("%i\n",expression(exprArr));
    }
    {
        char exprArr[100] = {0};
        sprintf(exprArr,"1+10*2)faszvagyok");
        printf("%i\n",expression(exprArr));
    }    
    {
        char exprArr[100] = {0};
        sprintf(exprArr,"1+x");
        subst(exprArr,"x",9);
        int ret = expression(exprArr);
        printf("%d\n",ret);
    }
    
    {
        char exprArr[100] = {0};
        sprintf(exprArr,"1+x+33");
        subst(exprArr,"x",1000);
        int ret = expression(exprArr);
        printf("%d\n",ret);
    }    
    
    {
        char exprArr[100] = {0};
        sprintf(exprArr,"1000+xanax");
        subst(exprArr,"xanax",1);
        int ret = expression(exprArr);
        printf("%d\n",ret);
    }        
   
    {
        int alma = -8;
        int ret = calcBuiltin("abs",alma);
        printf("%d\n",ret);
    }
    {
        char exprArr[100] = {0};
        sprintf(exprArr,"1+six(3)*2");
        
        calcSubFnc(exprArr,2);
        
        int ret = expression(exprArr);
        
        printf("%d\n",ret);
    }
    
    //printf("%i",result2);
    return 10;
}

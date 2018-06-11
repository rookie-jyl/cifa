#include<stdio.h>
#include<string.h>

#define MAX_STR 256
#define MAX_KEYWORDS 18
#define MAX_OPERATORS 13
#define MAX_SEPARATORS 6

/*分析状态*/
#define STA_START 1
#define STA_IDorKEYWORD 2  //IDENTIFIER： 标识符
#define STA_CONSTANT 3     //CONSTANT:    常量
#define STA_SINGLEOPE 4	   //OPERATOR:	  运算符
#define STA_DONE  5        //DONE：       完成

/* 定义所属类型 */
#define TYPE_KEYWORD 1     //KEYWORD：    保留字
#define TYPE_IDENTIFIER 2  //IDENTIFIER： 标识符
#define TYPE_CONSTANT 3    //CONSTANT:    常量
#define TYPE_OPERATOR 4    //OPERATOR:    运算符
#define TYPE_SEPARATOR 5   //SEPARATOR:   界限符
#define TYPE_ERROR 6       //ERROR:       错误
#define TYPE_UNKNOWN 7     //UNKNOWN:     未知
#define TYPE_ENDFILE 8     //ENDFILE:     文件结束

char *Operators[MAX_OPERATORS] = {"+","-","*","/",">",">=","<","<=","==","!=","&&","||","!"};
char *Separators[MAX_SEPARATORS] ={"{","}","(",")",";",","};
char *Keywords[MAX_KEYWORDS] = {"unsigned","break","return","void","case","float","char","for","while","continue","if","default","do","int","switch","double","long","else"};

/* 是否为运算符 */
int IsOperator(char c)
{
    int i;
    for(i=0;i<MAX_OPERATORS;i++)
        if(Operators[i][0]==c)
            return 1;
    return 0;
}
/* 是否为分隔符 */
int IsSeparator(char c)
{
    int i;
    for(i=0;i<MAX_SEPARATORS;i++)
        if(Separators[i][0]==c)
            return 1;
    return 0;
}
/* 是否为保留字 */
int IsKeyword(char *str)
{
    int i;
    for(i=0;i<MAX_KEYWORDS;i++)
        if(strcmp(Keywords[i],str)==0)
            return 1;
    return 0;
}
/* 到DOWN就输出*/
void OutputOneDone(FILE *outf,int type,char *str){
    if(IsKeyword(str)==1) type=TYPE_KEYWORD;//为保留字,type=1
    switch(type)
    {
        case TYPE_KEYWORD:    fprintf(outf,"       关键字:  ");break;
        case TYPE_IDENTIFIER: fprintf(outf,"       标志符:  ");break;
        case TYPE_CONSTANT:   fprintf(outf,"       常量:    ");break;
        case TYPE_OPERATOR:   fprintf(outf,"       运算符:  ");break;
        case TYPE_SEPARATOR:  fprintf(outf,"       界限符:  ");break;
        case TYPE_ERROR:      fprintf(outf,"       错误:    ");break;
        case TYPE_UNKNOWN:    fprintf(outf,"       未知:    ");break;
        default:break;
    }
    fprintf(outf,"%s\n",str);
}
/* DFA词法分析函数 */
void LexAnalyse(FILE *inf,FILE *outf){
	char c;
    char str[MAX_STR];//过程字符串
    int i;
    int line_no=1;//行号
    int state=1,type=1;
    char flag_had_got_dot;/* 用于限定小数中只能有一个小数点 */
	fprintf(outf,"Line %d--------------------------------------\n",line_no);
	while(!feof(inf)){
		i=0;
		state=STA_START;
		flag_had_got_dot=0;
		while(state!=STA_DONE){
			c=fgetc(inf);//c为读取的一个字符
			switch(state){
				case STA_START:
					if( c==' ' || c=='\t'){
						state=STA_START;
					}
					else if(c=='\n'){
						line_no++;
                    	fprintf(outf,"Line %d--------------------------------------\n",line_no);
					}
					else if((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_'){
						state=STA_IDorKEYWORD;//状态2
                    	type=TYPE_IDENTIFIER;//标识符
                    	str[i]=c;
                    	i++;
					}
					else if(c>='0' && c<='9'){
						state=STA_CONSTANT;//状态3
                    	type=TYPE_CONSTANT;//常量
                    	str[i]=c;
                    	i++;
					}
					else if(IsSeparator(c)){
						state=STA_DONE;
                     	type=TYPE_SEPARATOR;//界限符
                     	str[0]=c;
                     	i=1;
                    }
					else if(c=='+' || c=='-' || c=='*' || c=='/'){
						state=STA_DONE;
                     	type=TYPE_OPERATOR;//运算符
                     	str[0]=c;
                     	i=1;
					}
					else if(c=='<' || c=='>' || c=='!' || c=='='){
						state=STA_SINGLEOPE;
						type=TYPE_OPERATOR;//运算符
						str[0]=c;
                     	i=1;
					}
					else if(c==EOF){
                    	state=STA_DONE;
                    	type=TYPE_ENDFILE;//文件结束
                	}
                	else{
                    	state=STA_DONE;
                    	type=TYPE_UNKNOWN;//未知
                	}
					break;
				case  STA_IDorKEYWORD:
					if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9')){
						state=STA_IDorKEYWORD;//状态2
                    	type=TYPE_IDENTIFIER;//标识符
                    	str[i]=c;
                    	i++;
					}
					else if(c==' ' || c=='\t' || c=='\n' || IsOperator(c) || IsSeparator(c)){
                        state=STA_DONE;
                        fseek(inf, -1, SEEK_CUR);//文件流指针前移1个字节
                    }
					else{
						state=STA_DONE;
                    	type=TYPE_ERROR;
					}
					break;
				case STA_CONSTANT:
					if(c>='0' && c<='9'){
						state=STA_CONSTANT;//状态3
                    	type=TYPE_CONSTANT;//常量
                    	str[i]=c;
                    	i++;
					}
					else if(c=='.')//小数识别
                    {
                        str[i]=c;
                        i++;
                        c=fgetc(inf);
                        if(flag_had_got_dot==0)
                        {
                            if(c>='0' && c<='9')
                            {
                                str[i]=c;
                                i++;
                                flag_had_got_dot=1;
                            }
                            else
                            {
                                fseek(inf, -1, SEEK_CUR);//文件流指针前移1个字节
                                state=STA_DONE;
                                type=TYPE_ERROR;
                            }
                        }
                        else
                        {
                            type=TYPE_ERROR;
                            str[i]=c;
                            i++;
                        }
                    }
					else if(c==' ' || c=='\t' || c=='\n' || IsOperator(c) || IsSeparator(c)){
                        state=STA_DONE;
                        fseek(inf, -1, SEEK_CUR);//文件流指针前移1个字节
                    }
					else{
						state=STA_DONE;
						type=TYPE_ERROR;
					}
					break;
				case STA_SINGLEOPE:
					if(c=='='){
						state=STA_DONE;
                    	type=TYPE_OPERATOR;//运算符
                    	str[i]=c;
                    	i++;
					}
					else{
                        state=STA_DONE;
                        fseek(inf, -1, SEEK_CUR);//文件流指针前移1个字节
                    }
					break;
				case STA_DONE:  break;
                default:        break;
			}
		}
		str[i]='\0';
        OutputOneDone(outf,type,str);//DONE一次输出一次
	}
}
int main()
{
	FILE *input,*output;
	if((input=fopen("input.c","r"))==NULL){
		printf("Cannot find the file!\nStrike any key to exit!\n");
        system("pause");
        exit(1);
	}
	else{
		output=fopen("output.txt","w");
		LexAnalyse(input,output);
		fprintf(output,"----------------------------------------------END OF FILE!\n");
        fclose(input);
        fclose(output);
        printf("Lexical Analyzer has finished the analyzation!\nFor more information please see the file output.txt.\n");
        system("pause");
	}
}


#include"MemLog.h"
#include"data_structure.h"

void err( char *str ){
	printf("%s ; %s\n",str,strerror(errno));
	exit(1);
}

int myFileCreateMem( const char *str ){
	FILE *frw = fopen("MemN.txt","r");
	FILE *fw = fopen("Member.txt", "a" );
	int n;

	// max Member
	fscanf(frw,"%d",&n);
	++ n;

	fclose( frw );
	frw = fopen("MemN.txt","w");
	fprintf( frw, "%d",n);

	// append
	fseek( fw, 0 ,SEEK_END );
	fprintf( fw,"%d %s\n",n,str);

	fclose( frw );
	fclose( fw );

	return n;
}

void myFileCreateLog( const char *str ){
	FILE *fw = fopen("Log.txt", "a" );

	// append
	fseek( fw, 0 ,SEEK_END );
	fprintf( fw,"%s\n",str);

	fclose( fw );
}

struct user_data *myFileRetrieve( const char *id, const char *pw ){
	int i,n;
	char buf[maxstr];
	struct user_data *mem=NULL;
	FILE *fr = fopen("MemN.txt","r");

	fscanf(fr,"%d",&n);
	fclose( fr );
	fr = fopen("Member.txt","r");

	for( i = 0 ; i < n ; ++ i ){
		// id
		fscanf( fr, "%*d %s", buf );

		if( strcmp( id, buf ) ){
			fscanf( fr,"%*s %*d %*d");
			continue;
		}
		mem = (struct user_data *)calloc( 1, sizeof(struct user_data) );
		strcpy( mem->id, id );

		// pw
		if( pw == NULL ) break;
		fscanf( fr, "%s", buf );
		if( strcmp( pw, buf ) ){
			fscanf( fr,"%*d %*d");
			continue;
		}
		strcpy( mem->password, pw );

		// check OK
		fscanf( fr, "%ld",&mem->character_type);
		fscanf( fr, "%ld",&mem->exp);

		break;
	}

	fclose( fr );
	return mem;
}

void myFileUpdate( const int pk, const int exp ){
	int i,n,curPK;
	char buf[maxstr];
	FILE *tmpf = fopen("tmp.txt","w");
	FILE *fwr = fopen("MemN.txt","r");
	struct user_data *mem;

	fscanf( fwr, "%d",&n);
	fclose( fwr );
	fwr = fopen("Member.txt","r");

	// Member -> tmp
	for( i = 0 ; i < n ; ++ i ){
		// pk
		fscanf( fwr, "%ld",&curPK);
		if( curPK != pk ){
			fscanf( fwr, " %[^\n]",buf);
			fprintf( tmpf, "%ld %s\n",curPK,buf);
			continue;
		}

		// info
		mem = (struct user_data *)calloc( 1, sizeof(struct user_data) );
		mem->pk = curPK;
		fscanf( fwr, "%s",mem->id );
		fscanf( fwr, "%s",mem->password );
		fscanf( fwr, "%ld",&mem->character_type);
		fscanf( fwr, "%*d");

		// write
		fprintf( tmpf,"%ld %s %s %ld %ld\n",mem->pk,\
			mem->id,mem->password,mem->character_type,exp);
	}

	fclose( fwr );
	fclose( tmpf );

	// tmp -> Member
	fwr = fopen("Member.txt","w");
	tmpf = fopen("tmp.txt","r");

	while( fscanf( tmpf, "%[^\n]", buf ) != -1 ){
		fprintf( fwr, "%s\n", buf );
		fscanf( tmpf, "%*c");
	}

	fclose( fwr );
	fclose( tmpf );
}

int LoginMem( const char *id, const char *pw ){
	char str[maxstr];
	struct user_data *mem = myFileRetrieve( id, pw );
	if( mem == NULL ) // nothing id
		return -1;
	if( mem->character_type == 0 ) // wrong pw
		return 0;
	else
		return 1;
}

// Use Function

int RegMem( const char *id, const char *pw, const int img ){
	char str[maxstr];
	printf(" REGMEM\n");
	if( myFileRetrieve( id, NULL ) != NULL ) // exist id
		return 0;
	else{ // Success 
		printf("\n%s %s %d\n",id,pw,img);
		sprintf(str,"%s %s %d 0",id,pw,img);
		return myFileCreateMem( str );
	}
}

void UpdateExp( const int pk, const int exp ){
	myFileUpdate( pk, exp );
}

void PushLog( const char *cont ){
	char str[maxstr];
	time_t curTime;
	struct tm *stTime;
	time( &curTime );
	stTime = localtime( &curTime );

	sprintf( str, "[%4d.%02d.%02d %02d:%02d:%02d] %s",\
			stTime->tm_year+1900, stTime->tm_mon+1,stTime->tm_mday,\
			stTime->tm_hour, stTime->tm_min, stTime->tm_sec, cont);

	myFileCreateLog(str);
}


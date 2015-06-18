#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

void sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

char* getInsult()
{
  char *first_word[] = {"artless","bawdy","beslubbering","bootless","churlish","cockered","clouted","craven",
       "currish","dankish","dissembling","droning","errant","fawning","fobbing","froward","frothy","gleeking",
       "goatish","gorbellied","impertinent","infectious","jarring","loggerheaded","lumpish","mammering","mangled",
       "mewling","paunchy","pribbling","puking","puny","qualling","rank","reeky","roguish","ruttish","saucy",
       "spleeny","spongy","surly","tottering","unmuzzled","vain","venomed","villainous","warped","wayward",
       "weedy","yeasty","cullionly","fusty","caluminous","wimpled","burly-boned","misbegotten","odiferous",
       "poisonous","fishified","wart-necked"};

  char *second_word[] = {"base-court","bat-fowling","beef-witted","beetle-headed","boil-brained","clapper-clawed",
       "clay-brained","common-kissing","crook-pated","dismal-dreaming","dizzy-eyed","doghearted","dread-bolted",
       "earth-vexing","elf-skinned","fat-kidneyed","fen-sucked","flap-mouthed","fly-bitten","folly-fallen","fool-born",
       "full-gorged","guts-griping","half-faced","hasty-witted","hedge-born","hell-hated","idle-headed","ill-breeding",
       "ill-nurtured","knotty-pated","milk-livered","motley-minded","onion-eyed","plume-plucked","pottle-deep",
       "pox-marked","reeling-ripe","rough-hewn","rude-growing","rump-fed","shard-borne","sheep-biting","spur-galled",
       "swag-bellied","tardy-gaited","tickle-brained","toad-spotted","unchin-snouted","weather-bitten","whoreson"
       "malmsey-nosed","rampallian","lily-livered","scurvy-valiant","brazen-faced","unwash'd","bunch-back'd",
       "leaden-footed","muddy-mettled","pigeon-liver'd","scale-sided"};

  char *third_word[] = {"apple-john","baggage","barnacle","bladder","boar-pig","bugbear","bum-bailey","canker-blossom",
       "clack-dish","clotpole","coxcomb","codpiece","death-token","dewberry","flap-dragon","flax-wench","flirt-gill",
       "foot-licker","fustilarian","giglet","gudgeon","haggard","harpy","hedge-pig","horn-beast","hugger-mugger",
       "joithead","lewdster","lout","maggot-pie","malt-worm","mammet","measle","minnow","miscreant","moldwarp",
       "mumble-news","nut-hook","pigeon-egg","pignut","puttock","pumpion","ratsbane","scut","skainsmate","strumpet",
       "varlot","vassal","whey-face","wagtail","knave","blind-worm","popinjay","scullian","jolt-head","malcontent",
       "devil-monk","toad","rascal","basket-cockle"};

  char buffer[256];
  memset( buffer, 0, 256);
  int rand_num;

  // time(NULL), the typical random number seed, doesn't have enough
  // granularity if we are run multiple times in succession by a test.
  struct timeval tv;
  (void)gettimeofday(&tv, NULL);

  unsigned long seed = tv.tv_sec * 1000000 + tv.tv_usec;
  srandom(seed);
  //srand( time(NULL) );

  strcat( buffer, "You are a " );

  rand_num = rand() % (sizeof(first_word) / sizeof(*first_word));
  strcat( buffer, first_word[rand_num] );
  strcat( buffer, ", " );

  rand_num = rand() % (sizeof(first_word) / sizeof(*first_word));
  strcat( buffer, second_word[rand_num] );
  strcat( buffer, " " );

  rand_num = rand() % (sizeof(first_word) / sizeof(*first_word));
  strcat( buffer, third_word[rand_num] );
  strcat( buffer, "!\n" );

  return buffer;
}

int main(int argc, char *argv[])
{
  int sockfd, newfd;
  struct sockaddr_in server_in, client_in;
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;

  if( (sockfd = socket( PF_INET, SOCK_STREAM, 0)) == -1 ) {
    perror("socket");
    exit( 1 );
  }

  if( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  server_in.sin_family = AF_INET;
  server_in.sin_port = htons( 6666 );
  server_in.sin_addr.s_addr = INADDR_ANY;
  memset( &(server_in.sin_zero), '\0', 8);

  if( bind( sockfd, (struct sockaddr *)&server_in, sizeof(struct sockaddr)) == -1) {
    perror( "bind" );
    exit( 1 );
  }

  if( listen( sockfd, 20) == -1) {
    perror( "listen" );
    exit( 1);
  }

  sa.sa_handler = sigchld_handler;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = SA_RESTART;
  if( sigaction( SIGCHLD, &sa, NULL) == -1) {
    perror( "sigaction" );
    exit( 1 );
  }

  while( 1 ) {
    sin_size = sizeof( struct sockaddr_in);

    if( (newfd = accept( sockfd, (struct sockaddr *)&client_in, &sin_size)) == -1) {
      perror( "accept" );
      continue;
    }
    printf( "server: got connection from %s\n", inet_ntoa( client_in.sin_addr ));

    if( !fork() ) {
      char *insult = getInsult();

      close( sockfd );
      if( send( newfd, insult, strlen( insult ), 0) == -1) {
        perror( "send" );
      }
      close( newfd );
      exit( 0 );
    }
    close( newfd );
  }

  return 0;
}

// spreadCompareTestProgram.cpp
// ver 2.31
// 4/10 update

// データ量が増えると時間コストが爆発的に増加するため処理を分散させる
// 分散させる個数は100でハードコーディング
// 検索対象を固定せず 多対多

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include "math.h"
#include "time.h"

#define SPREAD_SIZE 100

using namespace std;
using namespace boost;

enum line_flags {ID_line, element_line, words_line, EOD};
typedef unordered_map<string,double> str_dbl_map;

struct Set{
  string ID;
  str_dbl_map VECTOR;
  double distance; //コサイン距離
  Set(string a=""){
    ID = a;
    distance = 0;
  }
};

bool GreaterDistance(const Set Left, const Set Right){
  return Left.distance > Right.distance;}

inline void stop(void);
inline Set setTweet(string &ID, double &element,vector<string> &words);

int main(int argc, char **argv){
  
  ifstream ifs(argv[1]); //log.txt
  string line,ID,rubbish;
  vector<string> words;
  double element;
  line_flags checker = ID_line;
  unsigned long long int counter=0,spread_order,spread_number;
  vector<Set> tweet_list;
  time_t start_time,end_time;

  time(&start_time);

  cerr << "100分割中何番目にしますか？" << endl;
  cin >> spread_order;

  cerr << "ツイート読み込み処理開始" <<endl;
  while( ifs && getline(ifs,line)){
    if(checker == ID_line){
      ID = line;
      checker = element_line;
    }
    else if(checker == element_line){
      element = atof(line.c_str());
      checker = words_line;
    }
    else if(checker == words_line){
      algorithm::split(words,line,is_space(),token_compress_on);
      words.pop_back(); //logの単語末尾に半角スペースがあるため
      checker = EOD;
    }
    else if(checker == EOD){
      Set temp = setTweet(ID,element,words);
      tweet_list.push_back(temp);      
      words.clear();
      checker = ID_line;
    }
    ++counter;
    if(counter % 100000 == 0)
      cerr << counter << " line finish" << endl;
  }
  cerr << "ツイート読み込み処理終了" << endl;

  const int tweet_count = tweet_list.size();
  cerr << "There are " <<tweet_count << " tweets" << endl;

  const double threshold_value = 0.6;
  cerr << "近似度が" << threshold_value << "以上の組を表示します" << endl;
  vector<Set> target_list = tweet_list;

  cerr << "ID1 \t\t\tID2\t\t\t近似度" <<endl;
  cerr << "===============================================================" << endl;

  spread_number = tweet_list.size()/SPREAD_SIZE;
  vector<Set>::iterator target=target_list.begin();
  target = target + spread_number * spread_order;
  vector<Set>::iterator target_end = target + spread_number;
  for(; target!=target_end; target++){ //組み合わせ開始
    for(vector<Set>::iterator tweet=tweet_list.begin(); tweet!=tweet_list.end(); tweet++){ //距離計算開始
      if( target->ID != tweet->ID){
	for(str_dbl_map::iterator it=target->VECTOR.begin(); it!=target->VECTOR.end(); it++){
	  string key = it->first;
	  const str_dbl_map::iterator itv = tweet->VECTOR.find(key);
	  if (itv != tweet->VECTOR.end()){
	    tweet->distance += it->second * itv->second;
	  }
	}
      }
    }

    sort(tweet_list.begin(), tweet_list.end(), GreaterDistance);
    
    const int RENGE = 30;
    for(int i=0; i<RENGE; ++i){
      if(tweet_list[i].distance >= threshold_value){
	cout << target->ID << "\t" << tweet_list[i].ID << "\t";
	cout << setprecision(12) << fixed << tweet_list[i].distance << endl;
      }
    }
    for(vector<Set>::iterator it=tweet_list.begin(); it!=tweet_list.end(); ++it)
      it->distance = 0;
  } //組み合わせ終了
  words.clear();
  tweet_list.clear();
  target_list.clear();
  time(&end_time);
  cerr << "所要時間" << difftime(end_time, start_time) << "秒" << endl;
}

inline Set setTweet(string &ID, double &element,vector<string> &words){
  Set temp(ID);
  for(vector<string>::iterator it=words.begin(); it!=words.end(); ++it){
    if(temp.VECTOR.count(*it)){
      double &tmp_elmnt = temp.VECTOR[*it];
      tmp_elmnt = sqrt(tmp_elmnt * tmp_elmnt + element * element);
    }
    else temp.VECTOR[*it] = element;
  }
  return temp;
  
}
  

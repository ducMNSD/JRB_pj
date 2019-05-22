#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "jrb.h"
#include "btree.h"
#include "soundex.h"
#include <time.h>

GObject *window;
GObject *frame;
GtkWidget *about_dialog;
GObject *textView,*textViewAdd,*textViewEdit;
GObject *textSearch,*EntryAdd,*EntryEdit;
BTA * data = NULL;

GObject *adddialog,*editdialog;
GtkListStore *list;

time_t t_begin;
time_t t_end;
char t_string[500];
static char code[128] = { 0 };
JRB TEST_RESULT;
void beginTest(char * s) {
  printf("begin\n");
  strcpy(t_string, s);
  printf("begin\n");
  if (jrb_find_str(TEST_RESULT, s) == NULL) {
    jrb_insert_str(TEST_RESULT, strdup(s), new_jval_v(make_jrb()));
  }
 // printf("aaaa\n");
  t_begin = clock();
}
void endTest() {
  t_end = clock();
  int time = (int)(t_end - t_begin);
  printf("%s: %lfs.\n", t_string, (double)(time) / 1000000);
  JRB tmp = jrb_find_str(TEST_RESULT, t_string);
  tmp = (JRB)jval_v(jrb_val(tmp));
  jrb_insert_int(tmp, (int)(time), JNULL);
}
void thonKeHieuSuatTrungBinh() {
  JRB ptr, ptmp, subptr;
  long totaltime;
  int counttest;
  printf(" ________________________________________________________\n");
  printf("| Tên hàm        |        Thời gian chạy trung bình      |\n");
  printf("|________________|_______________________________________|\n");
  jrb_traverse(ptr, TEST_RESULT) {
    if (strcmp("Sửa từ", jval_s(ptr->key) ) == 0)
      printf("| %-19s|", jval_s(ptr->key));
    else printf("| %-18s|", jval_s(ptr->key));
    totaltime = counttest = 0;
    ptmp = (JRB)jval_v(jrb_val(ptr));
    jrb_traverse(subptr, ptmp) {
      totaltime += jval_i(subptr->key);
      counttest++;
    }
    printf("%19lfs%20s\n",  (double)totaltime / counttest / 1000000, "|" );
    printf("|________________|_______________________________________|\n");
  }
}


void set_textView_text(char * text) {
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
	if (buffer == NULL) {
		printf("Get buffer fail!");
		buffer = gtk_text_buffer_new(NULL);
	}
	gtk_text_buffer_set_text(buffer, text, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textView), buffer);
}

void find_word(GtkWidget * widget, gpointer Data) {
  int rsize;
  char key[50];
  char mean[5000];
  strcpy(key, gtk_entry_get_text(GTK_ENTRY(textSearch)));
  beginTest("Tìm kiếm");
  btsel(data,key,mean,5000,&rsize);
  endTest();
  if(rsize==0){
    set_textView_text("ko co tu ban tim kiem\nVui long nhan Tab neu ban muon tim tu gan giong\nHoac nhan nut them tu de bo sung tu dien\n");
  }
  else set_textView_text(mean);
  
}


const char* soundex(const char *s)
{
  static char out[5];
  char in[100];
  strcpy(in,s);
  SoundEx(out,in,4,1);
  return out;
}
int prefix(const char * big, const char * small) {
  int small_len = strlen(small);
  int big_len = strlen(big);
  int i;
  if (big_len < small_len)
    return 0;
  for (i = 0; i < small_len; i++)
    if (big[i] != small[i])
      return 0;
  return 1;
}

void jrb_to_list(JRB nextWordArray, int number) {
  GtkTreeIter Iter;
  JRB tmp;
  int sochia = number / 9;
  int max = 8;
  if (sochia == 0) sochia = 1;
  jrb_traverse(tmp, nextWordArray) {
    if ((number--) % sochia == 0)  {
      gtk_list_store_append(list, &Iter);
      gtk_list_store_set(list, &Iter, 0, jval_s(tmp->key), -1 );
      if (max-- < 1)
		return;
    }
  }
}

int insert_insoundexlist(char * soundexlist , char * newword,  char * word, char * soundexWord) {
  if (strcmp(soundexWord, soundex(newword)) == 0) {
    if (strcmp(newword, word) != 0) {
      strcat(soundexlist, newword);
      strcat(soundexlist, "\n");
      return 1;
    }
  }
  else
    return 0;
}
int commond_char( char * str1, char * str2, int start) {
	int i;
	int slen1 = strlen(str1);
	int slen2 = strlen(str2);
	int slen  = (slen1 < slen2) ? slen1 : slen2;
	for ( i = start; i < slen; i++)
		if (str1[i] != str2[i])
			return i;
	return i;
}
void suggest(char * word, gboolean Tab_pressed) {
  beginTest("Gợi ý");
  char nextword[100], prevword[100];
  int i, NumOfCommondChar, minNumOfCommondChar = 1000;
  int max;
  GtkTreeIter Iter;
  JRB tmp, nextWordArray = make_jrb();
  BTint value, existed = 0;
  strcpy(nextword, word);
  int wordlen = strlen(word);
  gtk_list_store_clear(list);
  if (bfndky(data, word, &value) ==  0) {
    existed = 1;
    gtk_list_store_append(list, &Iter);
    gtk_list_store_set(list, &Iter, 0, nextword, -1 );
  }
  if (!existed)
    btins(data, nextword, "", 1);

  for (i = 0; i < 1945; i++) {
    bnxtky(data, nextword, &value);
    if (prefix(nextword, word)) {
      jrb_insert_str(nextWordArray, strdup(nextword), JNULL);
    }
    else break;
  }

  if (!existed && Tab_pressed) {
    if (jrb_empty(nextWordArray)) {
    	//printf("trc \n");
      char soundexlist[1000] = "Ý bạn là:\n";
      char soundexWord[50];
      strcpy(nextword, word);
      strcpy(prevword, word);
      strcpy(soundexWord, soundex(word));
      max = 5;
      for (i = 0; (i < 10000 ) && max; i++) {
	if (bprvky(data , prevword, &value) == 0)
	  if (insert_insoundexlist(soundexlist, prevword, word, soundexWord))
	    max--;
      }
      max = 5;
      for (i = 0; (i < 10000 ) && max; i++) {
	if (bnxtky(data, nextword, &value) == 0)
	  if (insert_insoundexlist(soundexlist, nextword, word, soundexWord))
	    max--;
      }
      set_textView_text(soundexlist);
    }
    else {
    	//printf("sau\n");
      strcpy(nextword, jval_s(jrb_first(nextWordArray)->key));
      jrb_traverse(tmp, nextWordArray) {
	NumOfCommondChar = commond_char(nextword, jval_s(tmp->key), wordlen);
	if (minNumOfCommondChar > NumOfCommondChar)
	  minNumOfCommondChar = NumOfCommondChar;
      }

      if ((minNumOfCommondChar  != 1000) && (minNumOfCommondChar >= wordlen)) {

	nextword[minNumOfCommondChar+1] = '\0';
	//printf("min=%d\n",minNumOfCommondChar );

	gtk_entry_set_text(GTK_ENTRY(textSearch), nextword);
	gtk_editable_set_position(GTK_EDITABLE(textSearch), minNumOfCommondChar+1);
      }
    }

  }
  else

    jrb_to_list(nextWordArray, i);
  if (!existed)
    btdel(data, word);
  jrb_free_tree(nextWordArray);
  endTest();
}
gboolean on_key_down(GtkWidget * entry, GdkEvent * event, gpointer No_need) {
  GdkEventKey *keyEvent = (GdkEventKey *)event;
  char word[50];
  int len;
  strcpy(word, gtk_entry_get_text(GTK_ENTRY(textSearch)));
  //printf("%s\n",word );
  if (keyEvent->keyval == GDK_KEY_Tab) {
    //printf("suggest\n");
    suggest(word,  TRUE);
  }
  else {
    if (keyEvent->keyval != GDK_KEY_BackSpace) {
      len = strlen(word);
      word[len] = keyEvent->keyval;
      word[len + 1] = '\0';
    }
    else {
      len = strlen(word);
      word[len - 1] = '\0';
    }
    suggest(word, FALSE);
  }
  return FALSE;
}
void Show_message(GtkWidget * parent , GtkMessageType type,  char * mms, char * content) {
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 type,
	                                 GTK_BUTTONS_OK,
	                                 "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}

void Add_word_to_dict(GtkWidget * widget, gpointer Data ){
	GtkTextIter st_iter;
	GtkTextIter ed_iter;
	BTint x;
	int result;
	gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(textViewAdd)), &st_iter);
	gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(textViewAdd)), &ed_iter);
	gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textViewAdd)), &st_iter, &ed_iter, FALSE);
	char wordtext[100]; 
	strcpy(wordtext,gtk_entry_get_text(GTK_ENTRY(EntryAdd)));
	char * meantext =  gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textViewAdd)), &st_iter, &ed_iter, FALSE);
	
	if (wordtext[0] == '\0' || meantext[0] == '\0')
		Show_message(adddialog, GTK_MESSAGE_WARNING, "Cảnh báo!", "Không được bỏ trống phần nào.");
	else if (bfndky(data, wordtext, &x ) == 0)
		Show_message(adddialog, GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Từ vừa nhập đã có trong từ điển.");
	else
	{
		beginTest("Thêm từ");
		result = btins(data, wordtext, meantext, strlen(meantext) + 1);
		//printf(" result= %d\n",result );
		endTest();
		if ( result == 0)
			Show_message(adddialog, GTK_MESSAGE_INFO, "Thành công!", "Đã thêm từ vừa nhập vào từ điển.");
		else
			Show_message(adddialog, GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
	}
}
void Edit_word_in_dict(GtkWidget * widget, gpointer Array) {
  GtkTextIter st_iter;
  GtkTextIter ed_iter;
  BTint x;
  int result;
  gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(textViewEdit)), &st_iter);//Lay chi so dau buffer
  gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(textViewEdit)), &ed_iter);//Lay chi so cuoi buffer
  char * wordtext = (char*)gtk_entry_get_text(GTK_ENTRY(textSearch));//Lay noi dung o nhap tu tieng anh
  char * meantext =  gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textViewEdit)), &st_iter, &ed_iter, FALSE);// Lay toan bo text trong Textview(cho hienthi nghia)
  if (wordtext[0] == '\0' || meantext[0] == '\0') // Bo trong thi canh bao
    Show_message(editdialog, GTK_MESSAGE_WARNING, "Cảnh báo!",
		 "Không được bỏ trống phần nào.");
  else if (bfndky(data, wordtext, &x ) != 0)//Neu tim thay canh bao
    Show_message(editdialog, GTK_MESSAGE_ERROR, "Xảy ra lỗi!",
		 "Không tìm thấy từ này trong từ điển.");
  else {
    
    beginTest("Sửa từ");//Bat dau do
   
    result = btupd(data, wordtext, meantext, strlen(meantext) + 1);//Cap nhat la word va nghia
    endTest();//Ket thuc do
    if (result == 0)
      Show_message(editdialog, GTK_MESSAGE_INFO, "Thành công!",
		   "Đã cập nhật lại nghĩa của từ trong từ điển.");// Cap nhat thanh cong
    else//Cap nhat that bai
      Show_message(editdialog, GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
  }
}
void Delete_word_from_dict(char * word) {
  beginTest("Xóa từ");
  int result = btdel(data, word);
  endTest();
  char anu[100] = "Đã xóa từ ";
  if (result == 0)
    Show_message(window, GTK_MESSAGE_INFO, "Thành công!", strcat(strcat(anu, word), " khỏi từ điển"));
  else
    Show_message(window, GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
  set_textView_text("");
  gtk_entry_set_text(GTK_ENTRY(textSearch), "");
  gtk_widget_grab_focus(textSearch);
}
void destroy_something(GtkWidget * widget, gpointer gp) {
  gtk_widget_destroy(gp);
}
void Show_add_dialog(GtkWidget * widget, gpointer dialog) {
  
  GtkBuilder *builder;
  GError *error=NULL;
  builder = gtk_builder_new();
  if(gtk_builder_add_from_file(builder,"Addwindow.ui",&error)==0){
    g_printerr ("Error loading file: %s\n",error->message);
    g_clear_error(&error);
    return 1;
  }
  GObject *ApplyButton,*CancelButton,*scroll;
  GObject *label1,*label2,*fixed;
  adddialog=gtk_builder_get_object(builder,"window1");
  fixed=gtk_builder_get_object(builder,"fixed1");
  ApplyButton=gtk_builder_get_object(builder,"button2");
  CancelButton=gtk_builder_get_object(builder,"button1");
  scroll=gtk_builder_get_object(builder,"scrolledwindow1");
  textViewAdd=gtk_builder_get_object(builder,"textview1");
  EntryAdd=gtk_builder_get_object(builder,"entry1");
  GtkWidget * data_array[3];
  g_signal_connect(ApplyButton, "clicked", G_CALLBACK(Add_word_to_dict), NULL);
  g_signal_connect(CancelButton,"clicked",G_CALLBACK(destroy_something),adddialog);
}
void Show_edit_dialog(GtkWidget * widget, gpointer dialog){
  BTint x;
  if (gtk_entry_get_text(GTK_ENTRY(textSearch))[0] == 0 ||
      bfndky(data, (char*)gtk_entry_get_text(GTK_ENTRY(textSearch)), &x) != 0) {
    Show_message(window, GTK_MESSAGE_WARNING, "Cảnh báo:", "Từ vừa nhập không có trong từ điển!");
    return;
  }
  GtkBuilder *builder;
  GError *error=NULL;
  builder = gtk_builder_new();
  if(gtk_builder_add_from_file(builder,"Editdialog.ui",&error)==0){
    g_printerr ("Error loading file: %s\n",error->message);
    g_clear_error(&error);
    return 1;
  }
  GObject *ApplyButton,*CancelButton,*label3;
  editdialog=gtk_builder_get_object(builder,"window1");
  textViewEdit=gtk_builder_get_object(builder,"textview1");
  gtk_text_view_set_buffer(GTK_TEXT_VIEW(textViewEdit), gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView)));
  label3=gtk_builder_get_object(builder,"label3");
  gtk_label_set_text(GTK_LABEL(label3), gtk_entry_get_text(GTK_ENTRY(textSearch)));
  ApplyButton=gtk_builder_get_object(builder,"button1");
  CancelButton=gtk_builder_get_object(builder,"button2");
  g_signal_connect(ApplyButton, "clicked", G_CALLBACK(Edit_word_in_dict), NULL);
  g_signal_connect(CancelButton,"clicked",G_CALLBACK(destroy_something),editdialog);
}

void Show_delete_dialog(GtkWidget * widget, gpointer dialog) {
  BTint x;
  if (gtk_entry_get_text(GTK_ENTRY(textSearch))[0] == 0 ||
      bfndky(data, (char*)gtk_entry_get_text(GTK_ENTRY(textSearch)), &x) != 0) {
    Show_message(window, GTK_MESSAGE_WARNING, "Cảnh báo:", "Từ vừa nhập không có trong từ điển!");
    return;
  }
  GtkWidget *deldialog;
  deldialog = gtk_message_dialog_new(GTK_WINDOW(window),
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     GTK_MESSAGE_QUESTION,
				     GTK_BUTTONS_YES_NO,
				     "Xóa: \"%s\"?", gtk_entry_get_text(GTK_ENTRY(textSearch)));
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(deldialog), "Bạn thực sự muốn xóa từ \"%s\" chứ?",
					   gtk_entry_get_text(GTK_ENTRY(textSearch)));

  int result = gtk_dialog_run(GTK_DIALOG(deldialog));
  if (result == GTK_RESPONSE_YES)
    Delete_word_from_dict((char*)gtk_entry_get_text(GTK_ENTRY(textSearch)));
  gtk_widget_destroy(deldialog);
}

int main(int argc, char** argv){
  TEST_RESULT = make_jrb();
  data = btopn("AnhViet.dat", 0, 1);
  GtkBuilder *builder;
  GObject *SearchButton,*AddButton,*EditButton,*DelButton;
  GObject *scrolling,*label,*Close;
  GError *error=NULL;
  gtk_init(&argc,&argv);
  builder = gtk_builder_new();
  if(gtk_builder_add_from_file(builder,"Project.ui",&error)==0){
    g_printerr ("Error loading file: %s\n",error->message);
    g_clear_error(&error);
    return 1;
  }
  GtkEntryCompletion *comple;
  // khoi tao cac nut
  window=gtk_builder_get_object(builder,"window1");
  SearchButton=gtk_builder_get_object(builder,"search");
  AddButton=gtk_builder_get_object(builder,"ADD");
  EditButton=gtk_builder_get_object(builder,"Edit");
  DelButton=gtk_builder_get_object(builder,"Del");
  frame=gtk_builder_get_object(builder,"fixed");
  scrolling=gtk_builder_get_object(builder,"scrolledwindow1");
  textSearch=gtk_builder_get_object(builder,"searchentry1");
  textView=gtk_builder_get_object(builder,"textview1");
  label=gtk_builder_get_object(builder,"label1");
  Close=gtk_builder_get_object(builder,"button6");
  // khoi tao goi y
  comple = gtk_entry_completion_new();
  gtk_entry_completion_set_text_column(comple, 0);
  list = gtk_list_store_new(10, G_TYPE_STRING, G_TYPE_STRING,
			    G_TYPE_STRING, G_TYPE_STRING,
			    G_TYPE_STRING, G_TYPE_STRING,
			    G_TYPE_STRING, G_TYPE_STRING,
			    G_TYPE_STRING, G_TYPE_STRING);

  gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(list));
  gtk_entry_set_completion(GTK_ENTRY(textSearch), comple);
  //gan su kien
  
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (SearchButton,"clicked",G_CALLBACK(find_word),NULL);
  g_signal_connect(textSearch, "key-press-event",G_CALLBACK(on_key_down),NULL);
  g_signal_connect(AddButton,"clicked",G_CALLBACK(Show_add_dialog),NULL);
  g_signal_connect(EditButton,"clicked",G_CALLBACK(Show_edit_dialog),NULL);
  g_signal_connect(DelButton,"clicked",G_CALLBACK(Show_delete_dialog),NULL);
  g_signal_connect(Close,"clicked",G_CALLBACK(gtk_main_quit),NULL);
  g_signal_connect(textSearch, "activate", G_CALLBACK(find_word), NULL);

  gtk_main();
  thonKeHieuSuatTrungBinh();
  btcls(data);
	
  return 0;
}

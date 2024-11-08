#include <stdio.h>
#include <stdlib.h>
#include "streaming_service.h"



///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// FUNCTION DEFINITIONS ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*
 * Register User - Event R
 * 
 * Adds user with ID uid to
 * users list, as long as no
 * other user with the same uid
 * already exists.
 *
 * Returns 0 on success, -1 on
 * failure (user ID already exists,
 * malloc or other error)
 */

int register_user(int uid)
{
    // Run through the user_list to ensure that there are no user duplicates
    for(struct user* usr = User_List.root; usr->uid != -1; usr=usr->next)
    {
        if (usr->uid == uid)
        {
            return -1;
        }
    }

    struct user * new_user = (struct user *)malloc(sizeof(struct user));
    new_user->uid = uid;
    new_user->suggestedHead = NULL;
    new_user->suggestedTail = NULL;
    new_user->watchHistory = NULL;
    new_user->next = User_List.root;
    User_List.root = new_user;

    printf("R <%d>\n   Users = ",  uid);
    struct user *usr;
    for(usr = User_List.root; usr->next->uid != -1; usr=usr->next)
    {
        printf("<%d>, ", usr->uid);
    }
    printf("<%d>\nDONE\n\n", usr->uid);

    return 0;
}


int add_new_movie(unsigned mid, movieCategory_t category, unsigned year)
{
    // Check if List is empty
    if (New_Movie_List.root == NULL)
    {
        struct new_movie *new_mv = make_new_movie(mid, category, year);
        new_mv->next = NULL;
        New_Movie_List.root = new_mv;

        // Print Outputs
        printf("A <%d> <%d> <%d>\n   ", new_mv->info.mid, new_mv->category, new_mv->info.year);
        printf("New movies = <%d,%d,%d>\n\n", new_mv->info.mid, new_mv->category, new_mv->info.year);
        return 0;
    }

    
    // Chek if movie already exists, no duplicates on mid
    for (struct new_movie *new_mv = New_Movie_List.root; new_mv != NULL; new_mv = new_mv->next)
    {        
        if (new_mv->info.mid == mid) return -1; // Movie already exists
    }

    // Make new Movie
    struct new_movie *new_mv = make_new_movie(mid, category, year);

    // Check if the movie should be placed at the root node
    if (New_Movie_List.root->info.mid >= mid)
    {
        new_mv->next = New_Movie_List.root;
        New_Movie_List.root = new_mv;
    }
    else
    {
        // Run through the list to place movie at the right spot with respect to mid
        struct new_movie *new_mv_index;
        for (new_mv_index = New_Movie_List.root; new_mv_index->next != NULL && new_mv_index->next->info.mid < mid ; new_mv_index = new_mv_index->next);
        if (new_mv_index->next == NULL) new_mv->next = NULL;
        else  new_mv->next = new_mv_index->next;
        new_mv_index->next = new_mv;
    }



    // Print Outputs
    printf("A <%d> <%d> <%d>\n   New movies = ", new_mv->info.mid, new_mv->category, new_mv->info.year);
    for(struct new_movie* new_mv = New_Movie_List.root; new_mv != NULL; new_mv=new_mv->next)
    {
        if (new_mv->next == NULL) printf("<%d,%d,%d>\nDONE\n\n", new_mv->info.mid, new_mv->category, new_mv->info.year);
        else   printf("<%d,%d,%d>, ", new_mv->info.mid, new_mv->category, new_mv->info.year);

    }
    return 0;
}


void distribute_new_movies(void)
{
    // Check if New_Movie_List is empty
    if (New_Movie_List.root == NULL) 
    {
        printf("The New Movie List is empty.\n");
        return;
    }

    // Access new movies recursively
    distribute_movie(New_Movie_List.root);
    New_Movie_List.root = NULL;
    printf("D\nCategorized Movies:\n");
    for(int i=0; i<6; i++)
    {        
        printf("   ");
        switch (i)
        {
            case 0:
                printf("Horror: ");
                break;
            case 1:
                printf("Sci-fi: ");
                break;
            case 2:
                printf("Drama: ");
                break;
            case 3:
                printf("Romance: ");
                break;
            case 4:
                printf("Documentary: ");
                break;
            case 5:
                printf("Comedy: ");
            default:
                break;
        }
        if (Movie_Category_Array[i]->root == NULL)
        {
            printf("<>\n") ;
            continue;
        }  
        int j=0;
        struct movie* mv;
        for(mv = Movie_Category_Array[i]->root; mv->next != NULL; mv = mv->next)
        {
            printf("<%d,%d>, ", mv->info.mid, j);
            j++;
        }
        printf("<%d,%d>\n", mv->info.mid, j);
    }

}


int watch_movie(int uid, unsigned mid)
{
    // Find Movie
    struct movie_info mv_info;
    int result = get_movie_info(mid, &mv_info);
    if (result) return -1;

    // Find User
    struct user *usr;
    for(usr = User_List.root; usr->uid != -1 && usr->uid != uid; usr=usr->next);
    if(usr->uid == -1) return -1;

    // Movie and User identified
    struct movie *mv = make_movie(mid, mv_info.year);
    push_history_stack(mv, usr);


    // Print Output
    printf("W <%d> <%d>\n   Usr <%d> Watch History = ", uid, mid, uid);
    for (struct movie *mv = usr->watchHistory; mv!=NULL; mv=mv->next)
    {
        if(mv->next == NULL) printf("<%d>\nDONE\n\n", mv->info.mid);
        else printf("<%d>, ", mv->info.mid);
    }

}


int suggest_movies(int uid)
{
    // Flag that gets triggered when at least one movie for suggestion has been found
    int can_suggest = 0;

    // Return if User List is empty
    if (User_List.root == NULL) return -1;

    //Identify uid user
    struct user *curr_usr;
    for(curr_usr = User_List.root; curr_usr->uid != -1 && curr_usr->uid != uid; curr_usr = curr_usr->next);
    // User with id uid not found, return -1
    if (curr_usr->uid == -1) return -1;

    // Run through the User list 
    int j=0;
    for(struct user *usr=User_List.root; usr->uid!=-1; usr=usr->next)
    {
        if(usr->uid != uid)
        {
            if (usr->watchHistory != NULL)
            {
                can_suggest = 1;

                if(j % 2 == 0)
                {
                    struct suggested_movie *mv = make_suggested_movie(usr->watchHistory->info.mid, usr->watchHistory->info.year);
                    pop_history_stack(usr);
                    push_head_suggested(curr_usr, mv);
                }
                else
                {
                    struct suggested_movie *mv = make_suggested_movie(usr->watchHistory->info.mid, usr->watchHistory->info.year);
                    pop_history_stack(usr);
                    push_tail_suggested(curr_usr, mv);
                }
            }
            
        }
    }


    if(can_suggest)
    {
        // Print output
        printf("S <%d>\n   User <%d> Suggested Movies = ", uid, uid);
        for(struct suggested_movie *mv = curr_usr->suggestedHead; mv != NULL; mv=mv->next)
        {
            if(mv->next == NULL) printf("<%d>\nDONE\n\n", mv->info.mid);
            else printf("<%d>, ", mv->info.mid);
        }
        return 0;
    }
    else return -1;
}


void unregister_user(int uid)
{
    // Check if uid refers to root
    if (User_List.root->uid == uid) 
    {
        struct user *usr = User_List.root;
        delete_suggested_movies(usr);
        delete_watch_history(usr);
        User_List.root = usr->next;
        free(usr);

        // Print Output
        printf("U <%d>\n   User = ", uid);
        for(struct user* usr = User_List.root; usr->uid != -1; usr=usr->next)
        {
            if (usr->next->uid == -1) printf("<%d>\nDONE\n\n", usr->uid);
            else printf("<%d>, ", usr->uid);
        }
        return;
    }

    // Uid does not refer to root
    struct user *prev_usr = get_prev_user(uid);
    if(!prev_usr) 
    {
        printf("User with uid:%d does not exist or is having an existential crisis\n", uid);
        return;
    }

    // Remove user elements and user from User_List
    struct user *usr = prev_usr->next;
    delete_suggested_movies(usr);
    delete_watch_history(usr);
    prev_usr->next = usr->next;
    free(usr);



    // Print output
    printf("U <%d>\n   User = ", uid);
    for(struct user* usr = User_List.root; usr->uid != -1; usr=usr->next)
    {
        if (usr->next->uid == -1) printf("<%d>\nDONE\n\n", usr->uid);
        else printf("<%d>, ", usr->uid);
    }
    return;
}


int filtered_movie_search(int uid, movieCategory_t category1,movieCategory_t category2, unsigned year)
{
    struct suggested_movie *head, *tail, *suggested_mv;
    head = NULL;
    tail = NULL;

    int can_filter = 0;

    // Check if category1 list is empty
    if (Movie_Category_Array[category1]->root != NULL)
    {
        // Run through all the Category movie list movies
        for(struct movie *mv = Movie_Category_Array[category1]->root; mv!=NULL; mv=mv->next)
        {
            // Condition on movie year
            if(mv->info.year >= year)
            {
                can_filter = 1;
                // Make new suggested_movie object
                suggested_mv = make_suggested_movie(mv->info.mid, mv->info.year);

                // Check if new dll is empty
                if (head == NULL)
                {
                    head = suggested_mv;
                    tail = suggested_mv;
                }
                else
                {
                    // Push on head
                    suggested_mv->next = head;
                    head->prev = suggested_mv;
                    head = suggested_mv;
                }
            }
        }
    }

    // Check if category2 list is empty
    if (Movie_Category_Array[category2]->root != NULL)
    {
        // Run through all the Category movie list movies
        for(struct movie *mv = Movie_Category_Array[category2]->root; mv!=NULL; mv=mv->next)
        {
            // Condition on movie year
            if(mv->info.year >= year)
            {
                can_filter = 1;
                // Make new suggested_movie object
                suggested_mv = make_suggested_movie(mv->info.mid, mv->info.year);

                // Check if new dll is empty
                if (head == NULL)
                {
                    head = suggested_mv;
                    tail = suggested_mv;
                }
                else
                {
                    // Push on head
                    suggested_mv->next = head;
                    head->prev = suggested_mv;
                    head = suggested_mv;
                }
            }
        }
    }

    if(!can_filter) return -1;

    // Connect generated DLL with DLL related to User:uid
    // Check if UserList is empty
    struct user *usr = get_user(uid);
    if(!usr) return -1;
    if(usr)
    {
        // Check if User suggested movie List is empty
        if(usr->suggestedHead != NULL)
        {
            usr->suggestedHead->prev = tail;
            tail->next = usr->suggestedHead;
            usr->suggestedHead = head;
        }
        else
        {
            usr->suggestedHead = head;
            usr->suggestedTail = tail;
        }
        
    }


    // Print Output
    printf("F <%d> <%d> <%d> <%d>\n   User <%d> Suggested Movies = ", uid, category1, category2, year, uid);
    for(struct suggested_movie *suggested_mv = usr->suggestedHead; suggested_mv!=NULL; suggested_mv=suggested_mv->next)
    {
        if(suggested_mv->next == NULL) printf("<%d>\nDONE\n\n", suggested_mv->info.mid);
        else printf("<%d>, ", suggested_mv->info.mid);
    }
    return 0;
}


void take_off_movie(unsigned mid)
{
    struct movie *mv;
    struct movie *helper_mv;
    movieCategory_t category;
    int movie_found = 0;

    // It is impossible given the way that code has been structured, for a movie to be in a user suggested list and not in a 
    // category list. Thus, we will try to find the movie in category lists first.
    for(int i=0; i<6; i++)
    {
        // If empty continue
        if (!Movie_Category_Array[i]->root) continue;

        // If one element list
        if(Movie_Category_Array[i]->root->next == NULL)
        {
            if (Movie_Category_Array[i]->root->info.mid == mid)
            {
                movie_found = 1;
                category = i;
                free(Movie_Category_Array[i]->root);
                Movie_Category_Array[i]->root = NULL;
                break;
            }
            continue;
        }

        // Check if mid is in root
        if (Movie_Category_Array[i]->root->info.mid == mid)
        {
            movie_found = 1;
            category = i;
            struct movie *mv_root = Movie_Category_Array[i]->root;
            Movie_Category_Array[i]->root = mv_root->next;
            free(mv_root);
            break;
        }


        for(mv = Movie_Category_Array[i]->root; mv->next!=NULL && mv->next->info.mid != mid; mv=mv->next);
        if(mv->next != NULL)
        {
            // If in tail
            if (mv->next->next == NULL)
            {
                movie_found = 1;
                category = i;
                free(mv->next);
                mv->next = NULL;
            }
            else
            {
                movie_found = 1;
                category = i;
                helper_mv = mv->next;
                mv->next = mv->next->next;
                free(helper_mv);
            }
            break;
        }
    }
    if(!movie_found) 
    {
        printf("Requested movie does not exist in the streaming service.\n\n");
        return;
    }

    printf("T <%d>\n   ", mid);

    // Check If User List is empty
    if(User_List.root->uid == -1)
    {
        printf("<%d> removed from <%d> category list.\n   ", mid, category);
        // Identify last movie category element mid
        struct movie *last_mv;
        for(last_mv = Movie_Category_Array[category]->root; last_mv->next != NULL; last_mv=last_mv->next);
        printf("Category list = <%d>, <%d>\nDONE\n\n", Movie_Category_Array[category]->root->info.mid, last_mv->info.mid);
        return;
    }

    // After having found the movie and User List not empty scan all the users
    for (struct user *usr = User_List.root; usr->uid != -1; usr=usr->next)
    {
        // For each user scan the suggested movies DLL (Note: there might be duplicates, so keep scanning till the end)
        remove_movie(usr, mid);
    }
    printf("<%d> removed from <%d> category list.\n   ", mid, category);
    if(Movie_Category_Array[category]->root == NULL)
    {
        printf("Category list = <EMPTY>\nDONE\n\n");
        return;
    }
    for(helper_mv = Movie_Category_Array[category]->root; helper_mv->next != NULL; helper_mv=helper_mv->next);
    printf("Category list = <%d>, <%d>\nDONE\n\n", Movie_Category_Array[category]->root->info.mid, helper_mv->info.mid);
    return;
}


void print_movies(void)
{
    printf("M\nCategorized Movies:");
    for(int i=0; i<6; i++)
    {
        printf("\n   ");
        switch (i)
        {
            case 0:
                printf("Horror: ");
                break;
            case 1:
                printf("Sci-fi: ");
                break;
            case 2:
                printf("Drama: ");
                break;
            case 3:
                printf("Romance: ");
                break;
            case 4:
                printf("Documentary: ");
                break;
            case 5:
                printf("Comedy: ");
            default:
                break;
        }

        if(Movie_Category_Array[i]->root == NULL) printf("EMPTY");
        else
        {
            for(struct movie *mv = Movie_Category_Array[i]->root; mv!=NULL; mv=mv->next)
            {
                if(mv->next == NULL) printf("<%d>", mv->info.mid);
                else printf("<%d>, ", mv->info.mid);
            }
        }
    }
    printf("\nDONE\n\n");
    return;
}


void print_users(void)
{
    printf("P\nUsers:");

    for(struct user *usr = User_List.root; usr->uid != -1; usr=usr->next)
    {
        printf("\n   ");
        printf("<%d>:\n      Suggested: ", usr->uid);
        if(usr->suggestedHead == NULL) printf("EMPTY");
        else
        {
            for(struct suggested_movie *suggested_mv = usr->suggestedHead; suggested_mv!=NULL; suggested_mv=suggested_mv->next)
            {
                if(suggested_mv->next == NULL) printf("<%d>", suggested_mv->info.mid);
                else printf("<%d>, ", suggested_mv->info.mid);
            }
        }
        printf("\n      Watch History: ");
        if(usr->watchHistory == NULL) printf("EMPTY");
        else
        {
            for(struct movie *mv = usr->watchHistory; mv!=NULL; mv=mv->next)
            {
                if(mv->next == NULL) printf("<%d>", mv->info.mid);
                else printf("<%d>, ", mv->info.mid);
            }
        }

    }
    printf("\nDONE\n\n");
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// HELPER FUNCTION DEFINITIONS /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void distribute_movie(struct new_movie *new_mv)
{
    // Recursive call
    if (new_mv->next != NULL) distribute_movie(new_mv->next);

    // Allocate struct movie memory
    struct movie* mv = make_movie(new_mv->info.mid, new_mv->info.year);
    movieCategory_t categ = new_mv->category;
    mv->next = Movie_Category_Array[categ]->root;
    Movie_Category_Array[categ]->root = mv; 
    free(new_mv);

    return;
}


void make_sentinel(struct user* sentinel)
{
	sentinel->uid = -1;
	sentinel->suggestedHead = NULL;
	sentinel->suggestedTail = NULL;
	sentinel->watchHistory = NULL;
	sentinel->next = NULL;
}

int init_user_list(struct user* sentinel, struct user_list* User_List)
{
    // Check if NULL and return -1
    if (User_List->root != NULL)  return -1;
    User_List->root = sentinel;
    return 0;
}

void delete_user_list(struct user_list* usr_lst)
{
    // Check if user_list is empty
    if (usr_lst->root == NULL)
    {
        free(usr_lst);
        return;
    }
    delete_user_list_iter(usr_lst->root);
    free(usr_lst);
    return;
}

void delete_user_list_iter(struct user * usr)
{
    if (usr->uid != -1 && usr != NULL) delete_user_list_iter(usr->next);
    // TODO: FREE ALL RELATIVE STRUCTS
    // a) WATCH HISTORY
    delete_watch_history(usr);
    delete_suggested_movies(usr);
    free(usr);
    return;
}

int init_new_movie_list(struct new_movie_list* mv_list)
{
    mv_list->root = NULL;
    return 0;
}

struct new_movie* make_new_movie(unsigned mid, movieCategory_t category, unsigned year)
{
    struct new_movie *new_mv = (struct new_movie*)malloc(sizeof(struct new_movie));
    new_mv->category = category;
    struct movie_info *mv_info  = (struct movie_info*)malloc(sizeof(struct movie_info));
    mv_info->mid = mid;
    mv_info->year = year;
    new_mv->info = *mv_info;
    
    return new_mv;
    // Only leave next to be filled in by add_movie function
}


struct movie* make_movie(unsigned mid, unsigned year)
{
    struct movie *mv = (struct movie*)malloc(sizeof(struct movie));
    struct movie_info *mv_info  = (struct movie_info*)malloc(sizeof(struct movie_info));
    mv_info->mid = mid;
    mv_info->year = year;
    mv->info = *mv_info;
    mv->next = NULL;
    return mv;
    // Only leave next to be filled in by add_movie function
}


void delete_new_movie_list(struct new_movie_list* new_mv_lst)
{
    // Check if List is empty
    if (new_mv_lst->root == NULL)
    {
        free(new_mv_lst);
        return;
    }
    delete_new_movie_list_iter(new_mv_lst->root);
    free(new_mv_lst);
    return;
}

void delete_new_movie_list_iter(struct new_movie *new_mv)
{
    if(new_mv->next != NULL) delete_new_movie_list_iter(new_mv->next);
    free(&(new_mv->info));
    free(new_mv);
    return;
}


int	init_movie_category_array(struct movie_list* mv_lst[], int size)
{
    for (int i=0; i<size; i++)
    {
        mv_lst[i] = (struct movie_list*)malloc(sizeof(struct movie_list));
        if (mv_lst[i] == NULL)
        {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
        init_movie_list(mv_lst[i]);
    }
}

void init_movie_list(struct movie_list *list) {
    list->root = NULL;
}

void delete_movie_category_array(struct movie_list* mv_lst[], int size)
{
    for (int i=0; i<size; i++)
    {
        free(mv_lst[i]);
    }
    return;
}

void delete_watch_history(struct user *usr)
{
    while(usr->watchHistory != NULL) pop_history_stack(usr);
}


void push_history_stack(struct movie* mv, struct user* usr)
{
    // Check if Stack is Empty
    if (usr->watchHistory == NULL)
    {
        usr->watchHistory = mv;
        mv->next = NULL;
        return;
    }

    // If not empty
    struct movie *temp_mv = usr->watchHistory;
    usr->watchHistory = mv;
    mv->next = temp_mv;
    return;
}


void pop_history_stack(struct user* usr)
{
    if (usr->watchHistory == NULL) return;

    struct movie *temp_mv = usr->watchHistory;
    usr->watchHistory = usr->watchHistory->next;
    free(temp_mv);

}


int get_movie_info(unsigned mid, struct movie_info* mv_info)
{    
    for (int i=0; i<6; i++)
    {
        struct movie *mv;
        for(mv = Movie_Category_Array[i]->root; mv != NULL && mv->info.mid != mid; mv=mv->next);
        if (mv != NULL)
        {
            mv_info->mid = mid;
            mv_info->year = mv->info.year;
            return 0;
        }
    }
    return -1;
}


void push_head_suggested(struct user *usr, struct suggested_movie *mv)
{
    // Check if user suggestion list is empty
    if (usr->suggestedHead == NULL)
    {
        usr->suggestedHead = mv;
        usr->suggestedTail = mv;
        return;
    }

    mv->next = usr->suggestedHead;
    usr->suggestedHead->prev = mv;
    usr->suggestedHead = mv;
    return;
}


void push_tail_suggested(struct user *usr, struct suggested_movie *mv)
{
    // Check if user suggestion list is empty
    if (usr->suggestedHead == NULL)
    {
        usr->suggestedHead = mv;
        usr->suggestedTail = mv;
        return;
    }

    mv->prev = usr->suggestedTail;
    usr->suggestedTail->next = mv;
    usr->suggestedTail = mv;
    return;
}


struct suggested_movie* make_suggested_movie(unsigned mid, unsigned year)
{
    struct suggested_movie *mv = (struct suggested_movie*)malloc(sizeof(struct suggested_movie));
    struct movie_info *mv_info  = (struct movie_info*)malloc(sizeof(struct movie_info));
    mv_info->mid = mid;
    mv_info->year = year;
    mv->info = *mv_info;
    mv->next = NULL;
    mv->prev = NULL;
    return mv;
    // Only leave next and prev to be managed by add_movie function
}

void delete_suggested_movies(struct user *usr)
{
    while(usr->suggestedHead != NULL) pop_tail_suggested(usr);
}

void pop_tail_suggested(struct user *usr)
{
    // Check if suggested movie list is Empty
    if (usr->suggestedHead == NULL) return;

    // Check if it only has one element
    if (usr->suggestedTail->prev == NULL)
    {
        free(usr->suggestedTail);
        usr->suggestedHead = NULL;
        usr->suggestedTail = NULL;
        return;
    }

    struct suggested_movie *temp_mv = usr->suggestedTail;
    usr->suggestedTail = usr->suggestedTail->prev;
    usr->suggestedTail->next = NULL;
    free(temp_mv);
    return;
}


struct user* get_user(unsigned uid)
{
    // Check if List is Empty
    if(User_List.root == NULL) return NULL;
    
    // Get user
    struct user *usr;
    for(usr=User_List.root; usr->uid != -1 && usr->uid != uid; usr=usr->next);
    if (usr->uid == -1) return NULL;
    else return usr;
}

struct user* get_prev_user(unsigned uid)
{
    // Check if List is Empty
    if(User_List.root == NULL) return NULL;
    
    // Check if uid refers to the root element, in which case return user with uid
    if(User_List.root->uid == uid) return User_List.root;

    // Get user
    struct user *usr;
    for(usr=User_List.root; usr->next->uid != -1 && usr->next->uid != uid; usr=usr->next);
    if (usr->next->uid == -1) return NULL;
    else return usr;
}

void remove_movie(struct user *usr, unsigned mid)
{
    if(usr->suggestedHead == NULL) return;
    remove_movie_iter(usr, mid);
    return;
}

void remove_movie_iter(struct user *usr, unsigned mid)
{
    if(usr->suggestedHead == NULL) return;
    struct suggested_movie *suggested_mv;

    for(suggested_mv = usr->suggestedHead; suggested_mv!=NULL && suggested_mv->info.mid != mid; suggested_mv=suggested_mv->next);
    if(suggested_mv != NULL)
    {
        // If movie is in head
        if(suggested_mv == usr->suggestedHead)
        {
            if(usr->suggestedHead != usr->suggestedTail)
            {
                usr->suggestedHead = suggested_mv->next;
                suggested_mv->next->prev = NULL;
                free(suggested_mv);
                printf("<%d> removed from <%d> suggested list.\n   ", mid, usr->uid);
            }
            else
            {
                usr->suggestedHead = NULL;
                usr->suggestedTail = NULL;
                free(suggested_mv);
                printf("<%d> removed from <%d> suggested list.\n   ", mid, usr->uid);
            }

        }
        // If movie is in tail
        else if (suggested_mv == usr->suggestedTail)
        {
            usr->suggestedTail = suggested_mv->prev;
            suggested_mv->prev->next = NULL;
            free(suggested_mv);
            printf("<%d> removed from <%d> suggested list.\n   ", mid, usr->uid);
        }
        else
        {
            suggested_mv->next->prev = suggested_mv->prev;
            suggested_mv->prev->next = suggested_mv->next;
            free(suggested_mv);
            printf("<%d> removed from <%d> suggested list.\n   ", mid, usr->uid);
        }

        // Call recursive function again
        remove_movie_iter(usr, mid);
    }

    return;
}



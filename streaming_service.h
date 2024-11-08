/*
 * ============================================
 * file: streaming_service.h
 * @Author John Malliotakis (jmal@csd.uoc.gr)
 * @Version 23/10/2023
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Structure and function declarations
 *        for CS240 Project Phase 1,
 *        Winter Semester 2023-2024
 * ============================================
 */

#ifndef __CS240_STREAMING_SERVICE_H__

#define __CS240_STREAMING_SERVICE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Εxternals ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern struct user sentinel_node;
extern struct user_list User_List;
extern struct new_movie_list New_Movie_List;
extern struct movie_list* Movie_Category_Array[];
extern struct movie_history_stack Movie_History_Stack;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Data Structure Declarations ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct user_list {
	struct user *root;
};

struct new_movie_list {
	struct new_movie *root;
};

struct movie_list {
	struct movie *root;
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Node Structure Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	HORROR,
	SCIFI,
	DRAMA,
	ROMANCE,
	DOCUMENTARY,
	COMEDY
} movieCategory_t;

struct movie_info {
	unsigned mid;
	unsigned year;
};

struct movie {
	struct movie_info info;
	struct movie *next;
};

struct new_movie {
	struct movie_info info;
	movieCategory_t category;
	struct new_movie *next;
};

struct suggested_movie {
	struct movie_info info;
	struct suggested_movie *prev;
	struct suggested_movie *next;
};

struct user {
	int uid;
	struct suggested_movie *suggestedHead;
	struct suggested_movie *suggestedTail;
	struct movie *watchHistory;
	struct user *next;
};

// Declaration of movie genre array, as an array of struct movie pointers
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
int register_user(int uid);

/*
 * Unregister User - Event U
 *
 * Removes user with ID uid from
 * users list, as long as such a
 * user exists, after clearing the
 * user's suggested movie list and
 * watch history stack
 */
void unregister_user(int uid);

/*
 * Add new movie - Event A
 *
 * Adds movie with ID mid, category
 * category and release year year
 * to new movies list. The new movies
 * list must remain sorted (increasing
 * order based on movie ID) after every
 * insertion.
 *
 * Returns 0 on success, -1 on failure
 */
int add_new_movie(unsigned mid, movieCategory_t category, unsigned year);

/*
 * Distribute new movies - Event D
 *
 * Distributes movies from the new movies
 * list to the per-category sorted movie
 * lists of the category list array. The new
 * movies list should be empty after this
 * event. This event must be implemented in
 * O(n) time complexity, where n is the size
 * of the new movies list
 */
void distribute_new_movies(void);

/*
 * User watches movie - Event W
 *
 * Adds a new struct movie with information
 * corresponding to those of the movie with ID
 * mid to the top of the watch history stack
 * of user uid.
 *
 * Returns 0 on success, -1 on failure
 * (user/movie does not exist, malloc error)
 */
int watch_movie(int uid, unsigned mid);

/*
 * Suggest movies to user - Event S
 *
 * For each user in the users list with
 * id != uid, pops a struct movie from the
 * user's watch history stack, and adds a
 * struct suggested_movie to user uid's
 * suggested movies list in alternating
 * fashion, once from user uid's suggestedHead
 * pointer and following next pointers, and
 * once from user uid's suggestedTail pointer
 * and following prev pointers. This event
 * should be implemented with time complexity
 * O(n), where n is the size of the users list
 *
 * Returns 0 on success, -1 on failure
 */
int suggest_movies(int uid);

/*
 * Filtered movie search - Event F
 *
 * User uid asks to be suggested movies
 * belonging to either category1 or category2
 * and with release year >= year. The resulting
 * suggested movies list must be sorted with
 * increasing order based on movie ID (as the
 * two category lists). This event should be
 * implemented with time complexity O(n + m),
 * where n, m are the sizes of the two category lists
 *
 * Returns 0 on success, -1 on failure
 */
int filtered_movie_search(int uid, movieCategory_t category1,
		movieCategory_t category2, unsigned year);

/*
 * Take off movie - Event T
 *
 * Movie mid is taken off the service. It is removed
 * from every user's suggested list -if present- and
 * from the corresponding category list.
 */
void take_off_movie(unsigned mid);

/*
 * Print movies - Event M
 *
 * Prints information on movies in
 * per-category lists
 */
void print_movies(void);

/*
 * Print users - Event P
 *
 * Prints information on users in
 * users list
 */
void print_users(void);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Other Helper Functions ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
* Makes a sentinel node 
*/
void make_sentinel(struct user*);
/*
* Initialize a user_list by simply adding the sentinel node
* Returns -1 if given list is not empty
*/
int init_user_list(struct user*, struct user_list*);
/*
* An initial function to extract the root user from the list and pass it to the recursive user_list delete function
* Also deletes user_list struct
*/

void delete_user_list(struct user_list*);

void delete_user_list_iter(struct user*);

int init_new_movie_list(struct new_movie_list*);

struct new_movie* make_new_movie(unsigned, movieCategory_t, unsigned );

void delete_new_movie_list(struct new_movie_list*);

void delete_new_movie_list_iter(struct new_movie *);

void distribute_movie(struct new_movie*);

int	init_movie_category_array(struct movie_list*[], int);

void init_movie_list(struct movie_list*);

struct movie* make_movie(unsigned, unsigned);

void delete_movie_category_array(struct movie_list*[], int);

void push_history_stack(struct movie*, struct user*);

void pop_history_stack(struct user*);

int get_movie_info(unsigned, struct movie_info*);

void delete_watch_history(struct user*);

void pop_tail_suggested(struct user*);

void delete_suggested_movies(struct user*);

struct suggested_movie* make_suggested_movie(unsigned, unsigned);

void push_tail_suggested(struct user*, struct suggested_movie*);

void push_head_suggested(struct user*, struct suggested_movie*);

struct user* get_user(unsigned);

struct user* get_prev_user(unsigned);

void remove_movie_iter(struct user*, unsigned);

void remove_movie(struct user*, unsigned);

#endif
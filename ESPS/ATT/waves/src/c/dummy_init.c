#include <Objects.h>
char    play_program[], inputname[], *basename(), *remove_reg_exp();
extern void newText_proc();
Pending_input new_files = {
    inputname,          /* results_to */
    NULL,           /* path_name_prefix */
    (Panel_item) NULL,      /* item */
    NULL,         /* list (files created during a run of waves) */
    (Canvas) NULL,      /* display window */
    newText_proc,       /* procedure to call on selection */
    FALSE,          /* destroy_on_select */
    "New Files",        /* banner */
    NULL,           /* next */
    NULL };         /* prev */
Pending_input *input_pending = &new_files;

char * remove_reg_exp(s)
register char *s;
{}

char *
cleaned_for_input(s)
     register char *s;
{
}

char *expand_name(out, in)
     char *out, *in;
{}

put_waves_signal(s)
   Signal         *s;
{}

void
menu_redoit(canvas, pwi, repaint_area)
     Canvas canvas;
     Pixwin *pwi;
     Rectlist *repaint_area;
{}

make_edited_filename(s, realname)
   Signal         *s;
   char           *realname;
{}

update_window_titles(s)
   Signal         *s;
{
}

get_view_segment(view, start, page_time)
     register View *view;
     register double *start, *page_time;
{
}

plot_view(v)
     View *v;
{}

Menu_list *
add_to_menu_list(li,cp)
     Menu_list **li;
     char *cp;
{}

clone_methods(so, si)
   Signal         *so, *si;
{}

time_to_index(s, time)
   register Signal *s;
   register double time;
{
}

int  use_dsp32;
int da_location;

generic_time_to_x(v,time)
     register View *v;
     register double time;
{}

generic_yval_to_y(v, yval)
    register View   *v;
    register double yval;
{
}

double generic_x_to_time(v,x)
     register View *v;
     register int x;
{}

double generic_y_to_yval(v, y)
     register View   *v;
     int    y;
{}

int generic_ord_to_y(v, val)
     register    View    *v;
    double  val;
{
}

generic_xy_to_chan(v,x,y)
     register View *v;
     int x, y;
{
}

plot_cursors(v,color)
    View    *v;
    int     color;
{
}

plot_markers(v, which)
    View        *v;
    int         which;  /* 0=>right; 1=>left; 2=>both */
{
}

plot_hmarkers(v, which)
    View        *v;
    int         which;  /* 0=>bottom; 1=>top; 2=>both */
{
}

new_f0_view(s, c)
     Signal *s;
     Canvas c;
{
}

View * new_spect_view (sig, type, canvas)
     Signal *sig;
     int type;
     Canvas canvas;
{
}

double  image_clip = 7.0, image_range = 40.0;

element_size(s, i)
    register Signal *s;
    int             i;
{
}

View *focus_view;

get_labeled_chan(s, str)
   Signal         *s;
   char           *str;
{
}

void operate_wave_scrollbar(v, event)
     View *v;
     Event *event;
{}

void menu_operate(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
}

void (*right_button_proc)() = menu_operate;

scope_to_number (str)
     char *str;
{}

Menuop *search_all_menus_but(excluded, item_name)
     char *excluded, *item_name;
{
}

Signal *
tag_insert_signal(s1,s2,time)
     Signal *s1, *s2;
     double time;
{
}

View *
tag_setup_view(s, c)
     Signal *s;
     Canvas c;
{
}

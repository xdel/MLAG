/* Copyright (c) 2014  Mellanox Technologies, Ltd. All rights reserved.
 *
 * This software is available to you under BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */ 

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "mlag_log.h"
#include "mlag_bail.h"
#include "mlag_events.h"
#include "mac_sync_events.h"
#include "mlag_common.h"
#include "lib_ctrl_learn_defs.h"
#include "lib_ctrl_learn.h"
#include "mlag_mac_sync_manager.h"
#include "mlag_mac_sync_flush_fsm.h"
#include "mlag_mac_sync_peer_manager.h"
#include "mlag_mac_sync_master_logic.h"
#include "mlag_master_election.h"
#include "lib_commu.h"
#include "mlag_comm_layer_wrapper.h"
#include "mlag_mac_sync_dispatcher.h"

#undef  __MODULE__
#define __MODULE__ MLAG_MAC_SYNC_FLUSH_FSM

/************************************************
 *  Local Defines
 ***********************************************/

/************************************************
 *  Local Macros
 ***********************************************/

/************************************************
 *  Local Type definitions
 ***********************************************/


/************************************************
 *  Global variables
 ***********************************************/

/************************************************
 *  Local variables
 ***********************************************/

static mlag_verbosity_t LOG_VAR_NAME(__MODULE__) = MLAG_VERBOSITY_LEVEL_NOTICE;

/************************************************
 *  Local function declarations
 ***********************************************/

static int all_peers_ack(mlag_mac_sync_flush_fsm *fsm,
                         struct fsm_event_base *evt);

static int is_flush_needed(mlag_mac_sync_flush_fsm *fsm,
                         struct fsm_event_base *evt);

/************************************************
 *  Function implementations
 ***********************************************/

/**
 *  This function sets module log verbosity level
 *
 *  @param verbosity - new log verbosity
 *
 * @return void
 */
void
mlag_mac_sync_flush_fsm_log_verbosity_set(mlag_verbosity_t verbosity)
{
    LOG_VAR_NAME(__MODULE__) = verbosity;
}

/**
 *  This function gets module log verbosity level
 *
 * @return void
 */
mlag_verbosity_t
mlag_mac_sync_flush_fsm_log_verbosity_get(void)
{
    return LOG_VAR_NAME(__MODULE__);
}

/************************************************
 *  Master Election FSM declarations
 ***********************************************/

#ifdef FSM_COMPILED

/*  declaration of FSM using pseudo language */

/* *********************************** */
stateMachine
{
    mlag_mac_sync_flush_fsm()

/* *********************************** */

    events {
        start_ev(void *msg)
        stop_ev()
        peer_ack(int peer_id)
        /* peer_status_change_ev(int peer_id, int state) */
    }

    state idle {
        default
        transitions {
            { start_ev, $ is_flush_needed(fsm,evt) $, wait_peers, on_start() }
            { start_ev, $ else $               , idle      ,  on_pass_to_idle() }
        }
    }
    state wait_peers {
        tm = 300000
             transitions {
            {stop_ev,     NULL,                        idle ,       on_pass_to_idle() }
            { peer_ack,   $ all_peers_ack(fsm, evt) $, idle ,       on_pass_to_idle() }
            { peer_ack,   $ else $,                 IN_STATE,       NULL }
                  /* { TIMER_EVENT, $fsm->timeouts_cnt < 2   $,  wait_peers,NULL } */
            { TIMER_EVENT , NULL,                     idle  ,       on_timer() }
         }
    }
 } /* end of stateMachine */

#endif

/*#$*/
 /* The code  inside placeholders  generated by tool and cannot be modifyed*/


/*----------------------------------------------------------------
            Generated enumerator for  Events of FSM   
---------------------------------------------------------------*/
enum mlag_mac_sync_flush_fsm_events_t{
    start_ev  = 0, 
    stop_ev  = 1, 
    peer_ack  = 2, 
 };

/*----------------------------------------------------------------
            Generated structures for  Events of FSM   
---------------------------------------------------------------*/
struct  mlag_mac_sync_flush_fsm_start_ev_t
{
    int  opcode;
    const char *name;
    void*  msg ; 
};

struct  mlag_mac_sync_flush_fsm_stop_ev_t
{
    int  opcode;
    const char *name;
};

struct  mlag_mac_sync_flush_fsm_peer_ack_t
{
    int  opcode;
    const char *name;
    int  peer_id ; 
};

/*----------------------------------------------------------------
           State entry/exit functions prototypes  
---------------------------------------------------------------*/
/*----------------------------------------------------------------
            Generated functions for  Events of FSM   
---------------------------------------------------------------*/
int   mlag_mac_sync_flush_fsm_start_ev(struct mlag_mac_sync_flush_fsm  * fsm,  void*  msg)
{
     struct mlag_mac_sync_flush_fsm_start_ev_t ev;
     ev.opcode  =  start_ev ;
     ev.name    = "start_ev" ;
     ev.msg = msg ;

     return fsm_handle_event(&fsm->base, (struct fsm_event_base *)&ev);
}
int   mlag_mac_sync_flush_fsm_stop_ev(struct mlag_mac_sync_flush_fsm  * fsm)
{
     struct mlag_mac_sync_flush_fsm_stop_ev_t ev;
     ev.opcode  =  stop_ev ;
     ev.name    = "stop_ev" ;

     return fsm_handle_event(&fsm->base, (struct fsm_event_base *)&ev);
}
int   mlag_mac_sync_flush_fsm_peer_ack(struct mlag_mac_sync_flush_fsm  * fsm,  int  peer_id)
{
     struct mlag_mac_sync_flush_fsm_peer_ack_t ev;
     ev.opcode  =  peer_ack ;
     ev.name    = "peer_ack" ;
     ev.peer_id = peer_id ;

     return fsm_handle_event(&fsm->base, (struct fsm_event_base *)&ev);
}
/*----------------------------------------------------------------
                 Reactions of FSM    
---------------------------------------------------------------*/
static  int on_start (mlag_mac_sync_flush_fsm  * fsm, int parameter, struct fsm_event_base *event);
static  int on_pass_to_idle (mlag_mac_sync_flush_fsm  * fsm, int parameter, struct fsm_event_base *event);
static  int on_timer (mlag_mac_sync_flush_fsm  * fsm, int parameter, struct fsm_event_base *event);


/*----------------------------------------------------------------
           State Dispatcher function  
---------------------------------------------------------------*/
static int mlag_mac_sync_flush_fsm_state_dispatcher( mlag_mac_sync_flush_fsm  * fsm, uint16 state, struct fsm_event_base *evt, struct fsm_state_base** target_state );
/*----------------------------------------------------------------
           Initialized  State classes of FSM  
---------------------------------------------------------------*/
 struct fsm_state_base            state_mlag_mac_sync_flush_fsm_idle; 
 struct fsm_state_base            state_mlag_mac_sync_flush_fsm_wait_peers; 


 struct fsm_state_base      state_mlag_mac_sync_flush_fsm_idle ={ "idle", mlag_mac_sync_flush_fsm_idle , SIMPLE, NULL, NULL, &mlag_mac_sync_flush_fsm_state_dispatcher, NULL, NULL };

 struct fsm_state_base      state_mlag_mac_sync_flush_fsm_wait_peers ={ "wait_peers", mlag_mac_sync_flush_fsm_wait_peers , SIMPLE, NULL, NULL, &mlag_mac_sync_flush_fsm_state_dispatcher, NULL, NULL };

static struct fsm_static_data static_data= {0,0,1,0,0 ,{ &state_mlag_mac_sync_flush_fsm_idle,  &state_mlag_mac_sync_flush_fsm_wait_peers, }
};

 static struct fsm_state_base * default_state = &state_mlag_mac_sync_flush_fsm_idle ;
/*----------------------------------------------------------------
           StateDispatcher of FSM  
---------------------------------------------------------------*/

static int mlag_mac_sync_flush_fsm_state_dispatcher( mlag_mac_sync_flush_fsm  * fsm, uint16 state, struct fsm_event_base *evt, struct fsm_state_base** target_state )
{
  int err=0;
  struct fsm_timer_event *event = (  struct fsm_timer_event * )evt;
  switch(state) 
  {

   case mlag_mac_sync_flush_fsm_idle : 
    {
      if ( event->opcode == start_ev )
      {
         SET_EVENT(mlag_mac_sync_flush_fsm , start_ev)
         if(  is_flush_needed(fsm,evt)  )
         {/*tr00:*/
            err = on_start( fsm, DEFAULT_PARAMS_D, evt);  /* Source line = 122*/ 
            *target_state =  &state_mlag_mac_sync_flush_fsm_wait_peers;
            return err;
         }
         else
         {/*tr01:*/
            err = on_pass_to_idle( fsm, DEFAULT_PARAMS_D, evt);  /* Source line = 123*/ 
            *target_state =  &state_mlag_mac_sync_flush_fsm_idle;
            return err;
         }
      }

    }break;

   case mlag_mac_sync_flush_fsm_wait_peers : 
    {
      if ( event->opcode == stop_ev )
      {
         SET_EVENT(mlag_mac_sync_flush_fsm , stop_ev)
         {/*tr10:*/
            err = on_pass_to_idle( fsm, DEFAULT_PARAMS_D, evt);  /* Source line = 129*/ 
            *target_state =  &state_mlag_mac_sync_flush_fsm_idle;
            return err;
         }
      }

      else if ( event->opcode == peer_ack )
      {
         SET_EVENT(mlag_mac_sync_flush_fsm , peer_ack)
         if(  all_peers_ack(fsm, evt)  )
         {/*tr11:*/
            err = on_pass_to_idle( fsm, DEFAULT_PARAMS_D, evt);  /* Source line = 130*/ 
            *target_state =  &state_mlag_mac_sync_flush_fsm_idle;
            return err;
         }
         else
         {/*tr12:*/
           fsm->base.reaction_in_state = 1;
             /* Source line = 131*/ 
            *target_state =  &state_mlag_mac_sync_flush_fsm_wait_peers;
            return err;
         }
      }

      else if (( event->opcode == TIMER_EVENT )&& (event->id == mlag_mac_sync_flush_fsm_wait_peers) )
      {
         {/*tr13:*/
            err = on_timer( fsm, DEFAULT_PARAMS_D, evt);  /* Source line = 133*/ 
            *target_state =  &state_mlag_mac_sync_flush_fsm_idle;
            return err;
         }
      }

    }break;

   default:
   break;

  }
  return FSM_NOT_CONSUMED;

}
/*----------------------------------------------------------------
              Constructor of FSM   
---------------------------------------------------------------*/
int mlag_mac_sync_flush_fsm_init(struct mlag_mac_sync_flush_fsm *fsm, fsm_user_trace user_trace, void * sched_func, void * unsched_func)
 {
       fsm->state_timer[mlag_mac_sync_flush_fsm_idle] = 0 ;
       fsm->state_timer[mlag_mac_sync_flush_fsm_wait_peers] = 300000 ;

       fsm_init(&fsm->base,  default_state, 2, fsm->state_timer, user_trace, sched_func, unsched_func, &static_data);

      return 0;
 }
 /*  per state functions*/
/*----------------------------------------------------------------
              Getters for each State   
---------------------------------------------------------------*/
tbool mlag_mac_sync_flush_fsm_idle_in(struct mlag_mac_sync_flush_fsm *fsm){
    return fsm_is_in_state(&fsm->base, mlag_mac_sync_flush_fsm_idle);
}
tbool mlag_mac_sync_flush_fsm_wait_peers_in(struct mlag_mac_sync_flush_fsm *fsm){
    return fsm_is_in_state(&fsm->base, mlag_mac_sync_flush_fsm_wait_peers);
}
/*----------------------------------------------------------------
                 Reactions of FSM  (within comments . user may paste function body outside placeholder region)  
---------------------------------------------------------------*/

/*static  int on_start (mlag_mac_sync_flush_fsm  * fsm, int parameter, struct fsm_event_base *event)
 {
   SET_EVENT(mlag_mac_sync_flush_fsm , start_ev) ; 
 }
static  int on_pass_to_idle (mlag_mac_sync_flush_fsm  * fsm, int parameter, struct fsm_event_base *event)
 {
   SET_EVENT(mlag_mac_sync_flush_fsm , start_ev) ; 
 }
static  int on_timer (mlag_mac_sync_flush_fsm  * fsm, int parameter, struct fsm_event_base *event)
 {
   SET_EVENT(mlag_mac_sync_flush_fsm , TIMER_EVENT) ; 
 }


*/
/*    8121*/

/*#$*/

/* Below code written by user */



/*
 *  This function verifies whether all peers acknowledged flush process
 *
 * @param[in/out] fsm   - pointer to the flush FSM instance
 * @param[in] event - pointer to the current event structure
 *
 * @return 1 when successful, otherwise 0
 */
static int
all_peers_ack(mlag_mac_sync_flush_fsm *fsm, struct fsm_event_base *event)
{
    int i;
    int res, ret = 1;
    SET_EVENT(mlag_mac_sync_flush_fsm, peer_ack)

    fsm->responded_peers[ev->peer_id] = 1;

    for (i = 0; i < MLAG_MAX_PEERS; i++) {
        mlag_mac_sync_master_logic_is_peer_enabled(i, &res);
        if (res && (fsm->responded_peers[i] == 0)) {
            ret = 0;
            break;
        }
    }
    return ret;
}

/*
 *  This function verifies whether need to perform flush
 *
 * @param[in/out] fsm   - pointer to the flush FSM instance
 * @param[in] event - pointer to the current event structure
 *
 * @return 1 when successful, otherwise 0
 */
static int
is_flush_needed(mlag_mac_sync_flush_fsm *fsm, struct fsm_event_base *event)
{
    int  ret = 1;
    int err = 0;
    UNUSED_PARAM(fsm);
    int need_flush = 0;
    struct mac_sync_flush_peer_sends_start_event_data * msg = NULL;
    SET_EVENT(mlag_mac_sync_flush_fsm , start_ev);

    msg =  (struct mac_sync_flush_peer_sends_start_event_data *)ev->msg;
    if(msg->gen_data.non_mlag_port_flush){
    	ret = 1;
    	goto bail; /* flush need */
    }
    err = mlag_mac_sync_peer_mngr_check_need_flush( &msg->gen_data.filter, &need_flush);
    if(err){
    	/*Log error*/
    	ret = 0;
    	goto bail;
    }
    ret = (need_flush)? 1:0;

 bail:
    return ret;
}





/*
 *  This function is reaction on FSM's event Start. Sends Flush command to all
 *  enabled peers
 *
 * @param[in/out] fsm   - pointer to the flush FSM instance
 * @param[in] event - pointer to the current event structure
 *
 * @return 0 when successful, otherwise ERROR
 */
int
on_start(mlag_mac_sync_flush_fsm  * fsm, int parameter,
         struct fsm_event_base *event)
{
    int i;
    int err = 0;
    int res, msg_size =0;

    UNUSED_PARAM(parameter);
    SET_EVENT(mlag_mac_sync_flush_fsm, start_ev);
    struct mac_sync_flush_master_sends_start_event_data * msg = NULL;
    /*fsm->timeouts_cnt =0;*/

    msg = (struct mac_sync_flush_master_sends_start_event_data *)ev->msg;
    /* modify only opcode and send message back to peer*/
    msg->opcode = MLAG_MAC_SYNC_GLOBAL_FLUSH_MASTER_SENDS_START_EVENT;
    msg_size =
    (msg->number_mac_params) *sizeof(struct mac_sync_mac_params) +
    sizeof(struct mac_sync_flush_master_sends_start_event_data);

	MLAG_LOG(MLAG_LOG_INFO, "num mac params=%d,  size=%d\n",
			   msg->number_mac_params,msg_size );
    for (i = 0; i < MLAG_MAX_PEERS; i++) {
        fsm->responded_peers[i] = 0;
        mlag_mac_sync_master_logic_is_peer_enabled(i, &res);
        if (res) {
            /*Send message to peer about local flush with flush type*/
            //memcpy(&msg.filter, &ev->type, sizeof(msg.filter));

            err = mlag_mac_sync_dispatcher_message_send(
                MLAG_MAC_SYNC_GLOBAL_FLUSH_MASTER_SENDS_START_EVENT,
                (void *)msg, msg_size, i, MASTER_LOGIC);
            MLAG_BAIL_ERROR(err);
        }
    }
bail:
    return err;
}

/*
 *  This function is reaction on FSM's Timer event.
 *
 * @param[in] fsm   - pointer to the flush FSM instance
 * @param[in] event - pointer to the timer event
 *
 * @return 0 when successful, otherwise ERROR
 */
int
on_timer(mlag_mac_sync_flush_fsm  * fsm, int parameter,
         struct fsm_event_base *event)
{
    int i;
    int err = 0, res;

    UNUSED_PARAM(parameter);

    SET_EVENT(mlag_mac_sync_flush_fsm, TIMER_EVENT);
    for (i = 0; i < MLAG_MAX_PEERS; i++) {

        mlag_mac_sync_master_logic_is_peer_enabled(i, &res);
        if (res && (fsm->responded_peers[i] == 0)) {
            MLAG_BAIL_ERROR_MSG(ECANCELED,
                                "Timeout in Global Flush process for peer %d \n",
                                i);
        }
    }

bail:
    err = mlag_mac_sync_master_logic_notify_fsm_idle(fsm->key,1);
    if (err != 0) {
        MLAG_LOG(MLAG_LOG_ERROR,
                 "on_timer error at mlag_mac_sync_master_logic_notify_fsm_idle err [%d]\n",
                 err);
    }
    return err;
}


/*
 *  This function verifies whether FSM is "busy" with flush
 *
 * @param[in] fsm   - pointer to the flush FSM instance
 * @param[out] busy - pointer to the timer event
 *
 * @return 0 when successful, otherwise ERROR
 */
int
mlag_mac_sync_flush_is_busy(mlag_mac_sync_flush_fsm *fsm, int *busy)
{
	int err =0;
	ASSERT(fsm);
	ASSERT(busy);

    *busy = 1;
    if (mlag_mac_sync_flush_fsm_idle_in(fsm)) {
        *busy = 0;
    }
  bail:
    return err;
}


static int
on_pass_to_idle(mlag_mac_sync_flush_fsm  * fsm, int parameter,
                struct fsm_event_base *event)
{
    int err = 0;
    UNUSED_PARAM(parameter);

   SET_EVENT(mlag_mac_sync_flush_fsm , stop_ev) ;

   err = mlag_mac_sync_master_logic_notify_fsm_idle(fsm->key, 0);
   MLAG_BAIL_ERROR(err);

 bail:
       return err;
 }


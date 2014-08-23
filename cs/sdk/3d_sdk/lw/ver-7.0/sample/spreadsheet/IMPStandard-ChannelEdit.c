/*
 * IMPStandardBanks-ChannelEdit.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 *  Channel Value
 */
int channel_types[] = { LWET_FLOAT, LWET_DISTANCE, LWET_ANGLE, LWET_PERCENT };

int ChannelValue_Ghost( int column, int row, LWItemID id ) {
  return ((chan_info->channelType( id ) == channel_types[ column - 1 ]) ? IMPGHOST_ENABLED : IMPGHOST_BLANK);
}

/*
 * ChannelValue_Envelope():
 *  Since this is a channel value, it is always enveloped, so just return
 *   the id and ignore all requests.
 */
LWChannelID *ChannelValue_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  value_channels[0] = id;
  value_channels[1] = NULL;

  return value_channels;
}

IMPColumn col_ChannelAll = {
  "All Channels",                            /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Channel Values",                          /* Comment                 */
  ChannelValue_Envelope,                     /* Envelope function       */
  NULL,                                      /* Ghosted function        */
  NULL,                                      /* Query function          */
  NULL,                                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_ChannelFloat = {
  "Float Channel",                           /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Channel Values as Floats",                /* Comment                 */
  ChannelValue_Envelope,                     /* Envelope function       */
  ChannelValue_Ghost,                        /* Ghosted function        */
  NULL,                                      /* Query function          */
  NULL,                                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_ChannelDistance = {
  "Distance Channel",                        /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Channel Values as Distances",             /* Comment                 */
  ChannelValue_Envelope,                     /* Envelope function       */
  ChannelValue_Ghost,                        /* Ghosted function        */
  NULL,                                      /* Query function          */
  NULL,                                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_ChannelAngle = {
  "Angle Channel",                           /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Channel Values as Angles",                /* Comment                 */
  ChannelValue_Envelope,                     /* Envelope function       */
  ChannelValue_Ghost,                        /* Ghosted function        */
  NULL,                                      /* Query function          */
  NULL,                                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_ChannelPercent = {
  "Percent Channel",                         /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Channel Values as Percents",              /* Comment                 */
  ChannelValue_Envelope,                     /* Envelope function       */
  ChannelValue_Ghost,                        /* Ghosted function        */
  NULL,                                      /* Query function          */
  NULL,                                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  The Bank
 */
IMPColumn *col_channelValues[] = {
  &col_ChannelAll,
  &col_ChannelFloat,
  &col_ChannelDistance,
  &col_ChannelAngle,
  &col_ChannelPercent,
  NULL };

IMPBank bank_channelValues = {
  MakeBankID( '_', 'C', 'H', 'V' ),          /* id:  Standard (_) CHannel Values                      */
  "Channel Values",                          /* Bank Title                                            */
  IMPBASE_CHANNEL,                           /* Item base type                                        */
  col_channelValues,                         /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

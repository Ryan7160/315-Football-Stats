//Project by Jeffrey Samuels, Ryan Flanagan, Shawn Swathwood, Jordan Jollief, and Charlie Estes
//Make sure you've installed libcurl. This is most easily done on Linux using 'sudo apt-get install libcurl4-openssl-dev'.
//When you compile it, use 'gcc master.c -lcurl'.


//TO DO: Implement JSON parser. Use data structures to handle parsed data. Create ASCII table to display information to user. Create interactive menu for user to input what stats, year, and team they want to look at.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "json.h"

//We will probably need another, smaller structure that holds different statistics about a team. This will need to be done post-parsing.

//Defines structure for holding the results of the API call. Build another function that uses what happens in here to parse it.
struct MemoryStruct
{
  char *memory;
  size_t size;
};

//This function stores the results from the API call into struct mem of type MemoryStruct
static size_t
WriteMemoryCallback (void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *) userp;

  char *ptr = realloc (mem->memory, mem->size + realsize + 1);
  if (!ptr)
    {
      printf ("not enough memory (realloc returned NULL)\n");
      return 0;
    }

  mem->memory = ptr;
  memcpy (&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  //printf("%s", mem->memory);

  return realsize;
}

//This function handles the API call. We will modify this later so that it can call a specific API endnode. We do this by changing the URL. We can implement this by giving the user an interactive menu.
//API being used is NFL Team Stats. 'https://rapidapi.com/DathanStoneDev/api/nfl-team-stats'

static char *
request ()
{

  struct MemoryStruct chunk;
  chunk.memory = malloc (1);
  chunk.size = 0;

  CURL *curl;
  CURLcode res;
  curl = curl_easy_init ();
  if (curl)
    {
      //define api call as GET and direct it to given URL
      curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, "GET");
      curl_easy_setopt (curl, CURLOPT_URL,
            "https://nfl-team-stats.p.rapidapi.com/v1/nfl-stats/teams/receiving-stats/offense/2019");
      curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);

      //use curl over https
      curl_easy_setopt (curl, CURLOPT_DEFAULT_PROTOCOL, "https");

      struct curl_slist *headers = NULL;

      //send api key in curl header to endnode can identify who is requesting
      headers =
    curl_slist_append (headers,
               "x-rapidapi-key: f214693c05mshcb4f9b9797a4754p1c6cc7jsn454debc36d9a");
      //define API host in header of curl
      headers =
    curl_slist_append (headers,
               "x-rapidapi-host: nfl-team-stats.p.rapidapi.com");

      curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);

      //send data to struct MemoryStruct
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &chunk);

      //now that parameters for curl are set, run curl
      res = curl_easy_perform (curl);

      //if curl function doesnt return success, say there was an error
      if (res != CURLE_OK)
    {
      fprintf (stderr, "curl_easy_perform() failed: %s\n",
           curl_easy_strerror (res));
    }
      else
    {
      printf ("%lu bytes retrieved\n", (unsigned long) chunk.size);
    }
    }
  //cleans up curl from memory
  curl_easy_cleanup (curl);
  free (chunk.memory);

  curl_global_cleanup ();

}

//We need an interactive menu where the user is able to choose what kinds of stats they want.
//Different API endpoint URLs are broken up by defense/offense and for the given year.
//If we want them to have the option to choose more filtering options, like which team they want to see the stats for, we're going to need to implement that manually with the parser.

//We need a way of passing the results of the request function to the Jannson parser

//Then we need to make to make the parser segregate information by team stats

//We need to then have data structures for handling each group of information to pass to the display stats function.


int main ()
{
  request ();
}
/**
 * @fn static JsonToken *json_allocJsonToken(JsonParser *parser, JsonToken *tokens, size_t tokenNum)
 * @brief Allocates a fresh unused token from the token pull.
 * @param parser
 * @param tokens
 * @param tokenNum
 */
static JsonToken *json_allocJsonToken(JsonParser *parser, JsonToken *tokens, size_t tokenNum)
{
    if (parser->toknext >= tokenNum) return NULL;
    JsonToken *tok = &tokens[parser->toknext++];
    tok->start = tok->end = -1;
    tok->size = 0;
    #ifdef json_PARENT_LINKS
    tok->parent = -1;
    #endif
    return tok;
}

/**
 * @fn static void json_fillToken(JsonToken *token, JsonType type, int start, int end)
 * @brief Fills token type and boundaries.
 * @param token
 * @param type
 * @param start
 * @param end
 */
static void json_fillToken(JsonToken *token, JsonType type, int start, int end)
{
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * @fn static JsonError json_parsePrimitive(JsonParser *parser, const char *js, JsonToken *tokens, size_t num_tokens)
 * @brief Fills next available token with JSON primitive.
 * @param parser
 * @param js
 * @param tokens
 * @param num_tokens
 */
static JsonError json_parsePrimitive(JsonParser *parser, const char *js, JsonToken *tokens, size_t num_tokens)
{
    JsonToken *token;
    int start;

    start = parser->pos;

    for (; js[parser->pos] != '\0'; parser->pos++)
    {
        switch (js[parser->pos]) 
        {
            #ifndef json_STRICT
            /* In strict mode primitive must be followed by "," or "}" or "]" */
            case ':':
            #endif
            case '\t': 
            case '\r': 
            case '\n': 
            case ' ':
            case ',': 
            case ']': 
            case '}':
                goto found;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127)
        {
            parser->pos = start;
            return JSON_ERROR_INVAL;
        }
    }
    #ifdef json_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSON_ERROR_PART;
    #endif

    found:
    token = json_allocJsonToken(parser, tokens, num_tokens);
    if (!token)
    {
        parser->pos = start;
        return JSON_ERROR_NOMEM;
    }
    json_fillToken(token, JSON_PRIMITIVE, start, parser->pos);
    #ifdef json_PARENT_LINKS
    token->parent = parser->toksuper;
    #endif
    parser->pos--;
    return JSON_SUCCESS;
}

/**
 * @fn static JsonError json_parseString(JsonParser *parser, const char *js, JsonToken *tokens, size_t num_tokens)
 * @brief Fills next token with JSON string.
 * @param parser
 * @param js 
 * @param tokens
 * @param num_tokens
 */
static JsonError json_parseString(JsonParser *parser, const char *js, JsonToken *tokens, size_t num_tokens)
{
    JsonToken *token;
    int start = parser->pos;

    parser->pos++;

    /* Skip starting quote */
    for (; js[parser->pos] != '\0'; parser->pos++)
    {
        char c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"')
        {
            token = json_allocJsonToken(parser, tokens, num_tokens);
            if (!token)
            {
                parser->pos = start;
                return JSON_ERROR_NOMEM;
            }
            json_fillToken(token, JSON_STRING, start+1, parser->pos);
            #ifdef json_PARENT_LINKS
            token->parent = parser->toksuper;
            #endif
            return JSON_SUCCESS;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\')
        {
            parser->pos++;
            switch (js[parser->pos])
            {
                /* Allowed escaped symbols */
                case '\"': 
                case '/': 
                case '\\': 
                case 'b':
                case 'f': 
                case 'r': 
                case 'n': 
                case 't':
                    break;
                /* Allows escaped symbol \uXXXX */
                case 'u':
                    /// \todo handle JSON unescaped symbol \\uXXXX
                    break;
                /* Unexpected symbol */
                default:
                    parser->pos = start;
                    return JSON_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSON_ERROR_PART;
}

/**
 * @fn JsonError json_parseJson(JsonParser *parser, const char *js, JsonToken *tokens, unsigned int num_tokens) 
 * @brief Parse JSON string and fill tokens.
 * @param parser
 * @param js
 * @param tokens
 * @param num_tokens
 */

JsonError json_parseJson(JsonParser *parser, const char *js, JsonToken *tokens, unsigned int num_tokens) 
{
    JsonError r;
    int i;
    JsonToken *token;

    for (; js[parser->pos] != '\0'; parser->pos++)
    {
        char c;
        JsonType type;

        c = js[parser->pos];
        switch (c)
        {
            case '{':
            case '[':
                token = json_allocJsonToken(parser, tokens, num_tokens);
                if (!token) return JSON_ERROR_NOMEM;
                if (parser->toksuper != -1)
                {
                    tokens[parser->toksuper].size++;
                    #ifdef json_PARENT_LINKS
                    token->parent = parser->toksuper;
                    #endif
                }
                token->type = (c == '{' ? JSON_OBJECT : JSON_ARRAY);
                token->start = parser->pos;
                parser->toksuper = parser->toknext - 1;
                break;
            case '}':
            case ']':
                type = (c == '}' ? JSON_OBJECT : JSON_ARRAY);
                #ifdef json_PARENT_LINKS
                if (parser->toknext < 1) return JSON_ERROR_INVAL;
                token = &tokens[parser->toknext - 1];
                for (;;)
                {
                    if (token->start != -1 && token->end == -1)
                    {
                        if (token->type != type) return JSON_ERROR_INVAL;
                        token->end = parser->pos + 1;
                        parser->toksuper = token->parent;
                        break;
                    }
                    if (token->parent == -1) break;
                    token = &tokens[token->parent];
                }
                #else
                for (i = parser->toknext - 1; i >= 0; i--)
                {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1)
                    {
                        if (token->type != type) return JSON_ERROR_INVAL;
                        parser->toksuper = -1;
                        token->end = parser->pos + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1) return JSON_ERROR_INVAL;
                for (; i >= 0; i--)
                {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1)
                    {
                        parser->toksuper = i;
                        break;
                    }
                }
                #endif
                break;
            case '\"':
                r = json_parseString(parser, js, tokens, num_tokens);
                if (r < 0) return r;
                if (parser->toksuper != -1) tokens[parser->toksuper].size++;
                break;
            case '\t':
            case '\r':
            case '\n':
            case ':':
            case ',':
            case ' ': 
                break;
            #ifdef json_STRICT
            /* In strict mode primitives are: numbers and booleans */
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 't':
            case 'f':
            case 'n':
            #else
            /* In non-strict mode every unquoted value is a primitive */
            default:
            #endif
                r = json_parsePrimitive(parser, js, tokens, num_tokens);
                if (r < 0) return r;
                if (parser->toksuper != -1) tokens[parser->toksuper].size++;
                break;

            #ifdef json_STRICT
            /* Unexpected char in strict mode */
            default:
                return JSON_ERROR_INVAL;
            #endif

        }
    }

    for (i = parser->toknext - 1; i >= 0; i--)
    {
        /* Unmatched opened object or array */
        if (tokens[i].start != -1 && tokens[i].end == -1) return JSON_ERROR_PART;
    }

    return JSON_SUCCESS;
}


/**
 * @fn void json_initJsonParser(JsonParser *parser)
 * @brief Creates a new parser based over a given buffer with an array of tokens available.
 * @param parser
 */
void json_initJsonParser(JsonParser *parser)
{
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}

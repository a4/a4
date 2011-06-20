/**
 * Results container Interface
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#ifndef RESULTS_H
#define RESULTS_H

class Results
{
    public:
        enum Type
        {
            ROOT,
            PROTO_BUF
        };

        Results(const Type &type);
        virtual ~Results();

        Type type() const;

        virtual void add(const Results &) = 0;

        virtual void print() const = 0;

    private:
        Type _type;
};

#endif
